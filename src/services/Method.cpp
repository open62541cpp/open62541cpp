#include "open62541pp/services/Method.h"

#ifdef UA_ENABLE_METHODCALLS

#include <cstddef>
#include <iterator>

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

#include "../open62541_impl.h"

namespace opcua::services {

template <>
std::vector<Variant> call(
    Server& server,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    UA_CallMethodRequest request{};
    request.objectId = objectId;
    request.methodId = methodId;
    request.inputArgumentsSize = inputArguments.size();
    request.inputArguments = const_cast<UA_Variant*>(asNative(inputArguments.data()));  // NOLINT

    using Result = TypeWrapper<UA_CallMethodResult, UA_TYPES_CALLMETHODRESULT>;
    const Result result = UA_Server_call(server.handle(), &request);
    throwIfBad(result->statusCode);
    for (size_t i = 0; i < result->inputArgumentResultsSize; ++i) {
        throwIfBad(result->inputArgumentResults[i]);  // NOLINT
    }

    return {
        std::make_move_iterator(result->outputArguments),
        std::make_move_iterator(result->outputArguments + result->outputArgumentsSize)  // NOLINT
    };
}

template <>
std::vector<Variant> call(
    Client& client,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    size_t outputSize{};
    UA_Variant* output{};
    const auto status = UA_Client_call(
        client.handle(),
        objectId,
        methodId,
        inputArguments.size(),
        asNative(inputArguments.data()),
        &outputSize,
        &output
    );
    std::vector<Variant> result(
        std::make_move_iterator(output),
        std::make_move_iterator(output + outputSize)  // NOLINT
    );
    UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
    throwIfBad(status);
    return result;
}

}  // namespace opcua::services

#endif
