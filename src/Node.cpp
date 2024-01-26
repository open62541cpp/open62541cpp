#include "open62541pp/Node.h"

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"

#include "open62541_impl.h"

namespace opcua {

template <>
bool Node<Client>::exists() noexcept {
    // create minimal request
    UA_ReadValueId item{};
    item.nodeId = *getNodeId().handle();
    item.attributeId = UA_ATTRIBUTEID_NODECLASS;
    UA_ReadRequest request{};
    request.timestampsToReturn = UA_TIMESTAMPSTORETURN_NEITHER;
    request.nodesToReadSize = 1;
    request.nodesToRead = &item;

    const ReadResponse response = UA_Client_Service_read(getConnection().handle(), request);
    if (response->responseHeader.serviceResult != UA_STATUSCODE_GOOD ||
        response->resultsSize != 1) {
        return false;
    }
    if (response->results->hasStatus && response->results->status != UA_STATUSCODE_GOOD) {
        return false;
    }
    return true;
}

template <>
bool Node<Server>::exists() noexcept {
    void* context{};
    const auto status = UA_Server_getNodeContext(getConnection().handle(), getNodeId(), &context);
    return (status == UA_STATUSCODE_GOOD);
}

}  // namespace opcua
