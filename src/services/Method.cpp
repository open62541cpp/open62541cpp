#include "open62541pp/services/Method.h"

#ifdef UA_ENABLE_METHODCALLS

#include "open62541pp/Server.h"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/types/Composed.h"

namespace opcua::services {

CallResponse call(Client& connection, const CallRequest& request) {
    return callAsync(connection, request, detail::SyncOperation{});
}

template <>
std::vector<Variant> call(
    Server& connection,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    UA_CallMethodRequest item = detail::createCallMethodRequest(objectId, methodId, inputArguments);
    CallMethodResult result = UA_Server_call(connection.handle(), &item);
    return detail::getOutputArguments(result);
}

template <>
std::vector<Variant> call(
    Client& connection,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    return callAsync(connection, objectId, methodId, inputArguments, detail::SyncOperation{});
}

}  // namespace opcua::services

#endif
