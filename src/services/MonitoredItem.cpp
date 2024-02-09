#include "open62541pp/services/MonitoredItem.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <algorithm>  // for_each_n
#include <cstddef>
#include <memory>
#include <type_traits>  // is_same_v
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/ClientContext.h"
#include "open62541pp/detail/Result.h"  // tryInvoke
#include "open62541pp/detail/ServerContext.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/Variant.h"

#include "../open62541_impl.h"

namespace opcua::services {

inline static void copyMonitoringParametersToNative(
    const MonitoringParameters& parameters, UA_MonitoringParameters& native
) {
    native.samplingInterval = parameters.samplingInterval;
    native.filter = parameters.filter;
    native.queueSize = parameters.queueSize;
    native.discardOldest = parameters.discardOldest;
}

template <typename T>
inline static void reviseMonitoringParameters(MonitoringParameters& parameters, const T& result) {
    // response type may be UA_MonitoredItemCreateResult or UA_MonitoredItemModifyResult
    parameters.samplingInterval = result->revisedSamplingInterval;
    parameters.queueSize = result->revisedQueueSize;
    parameters.filter = asWrapper<ExtensionObject>(result->filterResult);
}

uint32_t createMonitoredItemDataChange(
    Client& client,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    UA_MonitoredItemCreateRequest request{};
    request.itemToMonitor = *itemToMonitor.handle();
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    copyMonitoringParametersToNative(parameters, request.requestedParameters);

    auto context = std::make_unique<detail::MonitoredItemContext>();
    context->catcher = &opcua::detail::getExceptionCatcher(client);
    context->itemToMonitor = itemToMonitor;
    context->dataChangeCallback = std::move(dataChangeCallback);
    context->deleteCallback = std::move(deleteCallback);

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Client_MonitoredItems_createDataChange(
        client.handle(),
        subscriptionId,
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        request,
        context.get(),
        context->dataChangeCallbackNativeClient,
        context->deleteCallbackNative
    );
    throwIfBad(result->statusCode);
    reviseMonitoringParameters(parameters, result);

    const auto monitoredItemId = result->monitoredItemId;
    client.getContext().monitoredItems.insert(
        {subscriptionId, monitoredItemId}, std::move(context)
    );
    return monitoredItemId;
}

uint32_t createMonitoredItemDataChange(
    Server& server,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    DataChangeNotificationCallback dataChangeCallback
) {
    UA_MonitoredItemCreateRequest request{};
    request.itemToMonitor = *itemToMonitor.handle();
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    copyMonitoringParametersToNative(parameters, request.requestedParameters);

    auto context = std::make_unique<detail::MonitoredItemContext>();
    context->catcher = &opcua::detail::getExceptionCatcher(server);
    context->itemToMonitor = itemToMonitor;
    context->dataChangeCallback = std::move(dataChangeCallback);

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Server_createDataChangeMonitoredItem(
        server.handle(),
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        request,
        context.get(),
        context->dataChangeCallbackNativeServer
    );
    throwIfBad(result->statusCode);
    reviseMonitoringParameters(parameters, result);

    const auto monitoredItemId = result->monitoredItemId;
    server.getContext().monitoredItems.insert({0U, monitoredItemId}, std::move(context));
    return monitoredItemId;
}

uint32_t createMonitoredItemEvent(
    Client& client,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    UA_MonitoredItemCreateRequest request{};
    request.itemToMonitor = *itemToMonitor.handle();
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    copyMonitoringParametersToNative(parameters, request.requestedParameters);

    auto context = std::make_unique<detail::MonitoredItemContext>();
    context->catcher = &opcua::detail::getExceptionCatcher(client);
    context->itemToMonitor = itemToMonitor;
    context->eventCallback = std::move(eventCallback);
    context->deleteCallback = std::move(deleteCallback);

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Client_MonitoredItems_createEvent(
        client.handle(),
        subscriptionId,
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        request,
        context.get(),
        context->eventCallbackNative,
        context->deleteCallbackNative
    );
    throwIfBad(result->statusCode);
    reviseMonitoringParameters(parameters, result);

    const auto monitoredItemId = result->monitoredItemId;
    client.getContext().monitoredItems.insert(
        {subscriptionId, monitoredItemId}, std::move(context)
    );
    return monitoredItemId;
}

void modifyMonitoredItem(
    Client& client,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    MonitoringParameters& parameters
) {
    UA_MonitoredItemModifyRequest itemToModify{};
    itemToModify.monitoredItemId = monitoredItemId;
    copyMonitoringParametersToNative(parameters, itemToModify.requestedParameters);

    UA_ModifyMonitoredItemsRequest request{};
    request.subscriptionId = subscriptionId;
    request.timestampsToReturn = static_cast<UA_TimestampsToReturn>(parameters.timestamps);
    request.itemsToModifySize = 1;
    request.itemsToModify = &itemToModify;

    using Response =
        TypeWrapper<UA_ModifyMonitoredItemsResponse, UA_TYPES_MODIFYMONITOREDITEMSRESPONSE>;
    const Response response = UA_Client_MonitoredItems_modify(client.handle(), request);
    throwIfBad(response->responseHeader.serviceResult);
    if (response->resultsSize != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    auto* result = response->results;
    throwIfBad(result->statusCode);
    reviseMonitoringParameters(parameters, result);
}

void setMonitoringMode(
    Client& client, uint32_t subscriptionId, uint32_t monitoredItemId, MonitoringMode monitoringMode
) {
    UA_SetMonitoringModeRequest request{};
    request.subscriptionId = subscriptionId;
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    request.monitoredItemIdsSize = 1;
    request.monitoredItemIds = &monitoredItemId;

    using Response = TypeWrapper<UA_SetMonitoringModeResponse, UA_TYPES_SETMONITORINGMODERESPONSE>;
    const Response response = UA_Client_MonitoredItems_setMonitoringMode(client.handle(), request);
    throwIfBad(response->responseHeader.serviceResult);
    if (response->resultsSize != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    throwIfBad(*response->results);
}

void setTriggering(
    Client& client,
    uint32_t subscriptionId,
    uint32_t triggeringItemId,
    Span<const uint32_t> linksToAdd,
    Span<const uint32_t> linksToRemove
) {
    UA_SetTriggeringRequest request{};
    request.subscriptionId = subscriptionId;
    request.triggeringItemId = triggeringItemId;
    request.linksToAddSize = linksToAdd.size();
    request.linksToAdd = const_cast<uint32_t*>(linksToAdd.data());  // NOLINT
    request.linksToRemoveSize = linksToRemove.size();
    request.linksToRemove = const_cast<uint32_t*>(linksToRemove.data());  // NOLINT

    using Response = TypeWrapper<UA_SetTriggeringResponse, UA_TYPES_SETTRIGGERINGRESPONSE>;
    const Response response = UA_Client_MonitoredItems_setTriggering(client.handle(), request);
    throwIfBad(response->responseHeader.serviceResult);
    std::for_each_n(response->addResults, response->addResultsSize, throwIfBad);
    std::for_each_n(response->removeResults, response->removeResultsSize, throwIfBad);
}

void deleteMonitoredItem(Client& client, uint32_t subscriptionId, uint32_t monitoredItemId) {
    const auto status = UA_Client_MonitoredItems_deleteSingle(
        client.handle(), subscriptionId, monitoredItemId
    );
    throwIfBad(status);
}

void deleteMonitoredItem(Server& server, uint32_t monitoredItemId) {
    const auto status = UA_Server_deleteMonitoredItem(server.handle(), monitoredItemId);
    throwIfBad(status);
    server.getContext().monitoredItems.erase({0U, monitoredItemId});
}

}  // namespace opcua::services

#endif
