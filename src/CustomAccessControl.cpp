#include "CustomAccessControl.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>  // invoke
#include <string>
#include <string_view>
#include <utility>  // move

#include "open62541pp/AccessControl.h"
#include "open62541pp/Config.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Logger.h"
#include "open62541pp/Server.h"
#include "open62541pp/Session.h"
#include "open62541pp/Wrapper.h"  // asWrapper, asNative
#include "open62541pp/detail/helper.h"
#include "open62541pp/detail/traits.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/ExtensionObject.h"

#include "open62541_impl.h"

namespace opcua {

/* -------------------------------------- Native callbacks -------------------------------------- */

inline static CustomAccessControl& getContext(UA_AccessControl* ac) noexcept {
    assert(ac != nullptr);
    assert(ac->context != nullptr);
    return *static_cast<CustomAccessControl*>(ac->context);
}

inline static Server& getServer(UA_AccessControl* ac) noexcept {
    return getContext(ac).getServer();
}

template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
inline static const WrapperType& asWrapperRef(const NativeType* nativePtr) {
    static const WrapperType empty;
    return nativePtr == nullptr ? empty : asWrapper<WrapperType>(*nativePtr);
}

inline static Session getSession(UA_AccessControl* ac, const UA_NodeId* sessionId) noexcept {
    // session constructed with every call to access control
    // future optimization: store session object in sessionContext
    return {getServer(ac), asWrapperRef<NodeId>(sessionId)};
}

inline static AccessControlBase& getAccessControl(UA_AccessControl* ac) {
    auto* accessControl = getContext(ac).getAccessControl();
    assert(accessControl != nullptr);
    return *accessControl;
}

inline static void logException(
    UA_Server* server, std::string_view callbackName, std::string_view exceptionMessage
) {
    const auto message =
        std::string("Exception in access control callback ")
            .append(callbackName)
            .append(": ")
            .append(exceptionMessage);
    log(server, LogLevel::Warning, LogCategory::Server, message);
}

template <typename F, typename ReturnType = std::invoke_result_t<F>>
inline static auto invokeAccessCallback(
    UA_Server* server, std::string_view callbackName, ReturnType returnOnException, F&& fn
) noexcept {
    try {
        return std::invoke(fn);
    } catch (const std::exception& e) {
        logException(server, callbackName, e.what());
        return returnOnException;
    }
}

static UA_StatusCode activateSession(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_EndpointDescription* endpointDescription,
    const UA_ByteString* secureChannelRemoteCertificate,
    const UA_NodeId* sessionId,
    const UA_ExtensionObject* userIdentityToken,
    [[maybe_unused]] void** sessionContext
) {
    return invokeAccessCallback(server, "activateSession", UA_STATUSCODE_BADINTERNALERROR, [&] {
        auto session = getSession(ac, sessionId);
        const auto status = getAccessControl(ac).activateSession(
            session,
            asWrapperRef<EndpointDescription>(endpointDescription),
            asWrapperRef<ByteString>(secureChannelRemoteCertificate),
            asWrapperRef<ExtensionObject>(userIdentityToken)
        );
        if (status.isGood()) {
            getContext(ac).onSessionActivated(asWrapperRef<NodeId>(sessionId));
        }
        return status.get();
    });
}

static void closeSession(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext
) {
    try {
        auto session = getSession(ac, sessionId);
        getAccessControl(ac).closeSession(session);
        getContext(ac).onSessionClosed(asWrapperRef<NodeId>(sessionId));
    } catch (const std::exception& e) {
        logException(server, "closeSession", e.what());
    }
}

static UA_UInt32 getUserRightsMask(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    return invokeAccessCallback(server, "getUserRightsMask", uint32_t{}, [&] {
        auto session = getSession(ac, sessionId);
        return getAccessControl(ac).getUserRightsMask(session, asWrapperRef<NodeId>(nodeId)).get();
    });
}

static UA_Byte getUserAccessLevel(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    return invokeAccessCallback(server, "getUserAccessLevel", uint8_t{}, [&] {
        auto session = getSession(ac, sessionId);
        return getAccessControl(ac).getUserAccessLevel(session, asWrapperRef<NodeId>(nodeId)).get();
    });
}

static UA_Boolean getUserExecutable(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* methodId,
    [[maybe_unused]] void* methodContext
) {
    return invokeAccessCallback(server, "getUserExecutable", false, [&] {
        auto session = getSession(ac, sessionId);
        return getAccessControl(ac).getUserExecutable(session, asWrapperRef<NodeId>(methodId));
    });
}

static UA_Boolean getUserExecutableOnObject(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* methodId,
    [[maybe_unused]] void* methodContext,
    const UA_NodeId* objectId,
    [[maybe_unused]] void* objectContext
) {
    return invokeAccessCallback(server, "getUserExecutableOnObject", false, [&] {
        auto session = getSession(ac, sessionId);
        return getAccessControl(ac).getUserExecutableOnObject(
            session, asWrapperRef<NodeId>(methodId), asWrapperRef<NodeId>(objectId)
        );
    });
}

static UA_Boolean allowAddNode(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_AddNodesItem* item
) {
    return invokeAccessCallback(server, "allowAddNode", false, [&] {
        auto session = getSession(ac, sessionId);
        return getAccessControl(ac).allowAddNode(session, asWrapperRef<AddNodesItem>(item));
    });
}

static UA_Boolean allowAddReference(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_AddReferencesItem* item
) {
    return invokeAccessCallback(server, "allowAddReference", false, [&] {
        auto session = getSession(ac, sessionId);
        return getAccessControl(ac).allowAddReference(
            session, asWrapperRef<AddReferencesItem>(item)
        );
    });
}

static UA_Boolean allowDeleteNode(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_DeleteNodesItem* item
) {
    return invokeAccessCallback(server, "allowDeleteNode", false, [&] {
        auto session = getSession(ac, sessionId);
        return getAccessControl(ac).allowDeleteNode(session, asWrapperRef<DeleteNodesItem>(item));
    });
}

static UA_Boolean allowDeleteReference(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_DeleteReferencesItem* item
) {
    return invokeAccessCallback(server, "allowDeleteReference", false, [&] {
        auto session = getSession(ac, sessionId);
        return getAccessControl(ac).allowDeleteReference(
            session, asWrapperRef<DeleteReferencesItem>(item)
        );
    });
}

[[maybe_unused]] static UA_Boolean allowBrowseNode(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    return invokeAccessCallback(server, "allowBrowseNode", false, [&] {
        auto session = getSession(ac, sessionId);
        return getAccessControl(ac).allowBrowseNode(session, asWrapperRef<NodeId>(nodeId));
    });
}

#ifdef UA_ENABLE_SUBSCRIPTIONS
[[maybe_unused]] static UA_Boolean allowTransferSubscription(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* oldSessionId,
    [[maybe_unused]] void* oldSessionContext,
    const UA_NodeId* newSessionId,
    [[maybe_unused]] void* newSessionContext
) {
    return invokeAccessCallback(server, "allowTransferSubscription", false, [&] {
        auto oldSession = getSession(ac, oldSessionId);
        auto newSession = getSession(ac, newSessionId);
        return getAccessControl(ac).allowTransferSubscription(oldSession, newSession);
    });
}
#endif

#ifdef UA_ENABLE_HISTORIZING
static UA_Boolean allowHistoryUpdateUpdateData(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    UA_PerformUpdateType performInsertReplace,
    const UA_DataValue* value
) {
    return invokeAccessCallback(server, "allowHistoryUpdate", false, [&] {
        auto session = getSession(ac, sessionId);
        return getAccessControl(ac).allowHistoryUpdate(
            session,
            asWrapperRef<NodeId>(nodeId),
            static_cast<PerformUpdateType>(performInsertReplace),
            asWrapperRef<DataValue>(value)
        );
    });
}

static UA_Boolean allowHistoryUpdateDeleteRawModified(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    UA_DateTime startTimestamp,
    UA_DateTime endTimestamp,
    bool isDeleteModified
) {
    return invokeAccessCallback(server, "allowHistoryDelete", false, [&] {
        auto session = getSession(ac, sessionId);
        return getAccessControl(ac).allowHistoryDelete(
            session,
            asWrapperRef<NodeId>(nodeId),
            DateTime(startTimestamp),
            DateTime(endTimestamp),
            isDeleteModified
        );
    });
}
#endif

/* ---------------------------------------------------------------------------------------------- */

namespace detail {

void clear(UA_AccessControl& ac) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 1)
    if (ac.clear != nullptr) {
        ac.clear(&ac);
    }
#else
    if (ac.deleteMembers != nullptr) {
        ac.deleteMembers(&ac);
    }
#endif
    ac = UA_AccessControl{};
}

}  // namespace detail

void CustomAccessControl::setAccessControl(UA_AccessControl& ac) {
    if (getAccessControl() == nullptr) {
        return;
    }

    detail::clear(ac);

    ac.context = this;
    ac.userTokenPoliciesSize = userTokenPolicies_.size();
    ac.userTokenPolicies = asNative(userTokenPolicies_.data());
    ac.activateSession = activateSession;
    ac.closeSession = closeSession;
    ac.getUserRightsMask = getUserRightsMask;
    ac.getUserAccessLevel = getUserAccessLevel;
    ac.getUserExecutable = getUserExecutable;
    ac.getUserExecutableOnObject = getUserExecutableOnObject;
    ac.allowAddNode = allowAddNode;
    ac.allowAddReference = allowAddReference;
    ac.allowDeleteNode = allowDeleteNode;
    ac.allowDeleteReference = allowDeleteReference;
#if UAPP_OPEN62541_VER_GE(1, 1)
    ac.allowBrowseNode = allowBrowseNode;
#endif
#if UAPP_OPEN62541_VER_GE(1, 2) && defined(UA_ENABLE_SUBSCRIPTIONS)
    ac.allowTransferSubscription = allowTransferSubscription;
#endif
#ifdef UA_ENABLE_HISTORIZING
    ac.allowHistoryUpdateUpdateData = allowHistoryUpdateUpdateData;
    ac.allowHistoryUpdateDeleteRawModified = allowHistoryUpdateDeleteRawModified;
#endif
}

void CustomAccessControl::setAccessControl(
    UA_AccessControl& native, AccessControlBase& accessControl
) {
    userTokenPolicies_ = accessControl.getUserTokenPolicies();
    accessControl_ = &accessControl;
    setAccessControl(native);
}

void CustomAccessControl::setAccessControl(
    UA_AccessControl& native, std::unique_ptr<AccessControlBase> accessControl
) {
    userTokenPolicies_ = accessControl->getUserTokenPolicies();
    accessControl_ = std::move(accessControl);
    setAccessControl(native);
}

void CustomAccessControl::onSessionActivated(const NodeId& sessionId) {
    sessionIds_.insert(sessionId);
}

void CustomAccessControl::onSessionClosed(const NodeId& sessionId) {
    sessionIds_.erase(sessionId);
}

std::vector<Session> CustomAccessControl::getSessions() const {
    assert(server_ != nullptr);
    std::vector<Session> result;
    for (auto id : sessionIds_) {
        result.emplace_back(*server_, std::move(id));
    }
    return result;
}

void CustomAccessControl::setServer(Server& server) noexcept {
    server_ = &server;
}

Server& CustomAccessControl::getServer() noexcept {
    assert(server_ != nullptr);
    return *server_;
}

// NOLINTNEXTLINE, https://stackoverflow.com/questions/53946674/noexcept-visitation-for-stdvariant
AccessControlBase* CustomAccessControl::getAccessControl() noexcept {
    return std::visit(
        detail::Overload{
            [](AccessControlBase* ptr) { return ptr; },
            [](std::unique_ptr<AccessControlBase>& ptr) { return ptr.get(); }
        },
        accessControl_
    );
}

}  // namespace opcua
