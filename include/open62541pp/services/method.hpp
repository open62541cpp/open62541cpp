#pragma once

#include <utility>  // forward

#include "open62541pp/async.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/services/detail/async_transform.hpp"
#include "open62541pp/services/detail/client_services.hpp"
#include "open62541pp/services/detail/request_handling.hpp"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"

#ifdef UA_ENABLE_METHODCALLS

namespace opcua {
class Client;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup Method Method service set
 * Call (invoke) methods.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11
 * @ingroup Services
 * @{
 *
 * @defgroup Call Call service
 * This Service is used to call (invoke) a list of methods.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11.2
 * @{
 */

/**
 * Call server methods.
 * @param connection Instance of type Client
 * @param request Call request
 */
CallResponse call(Client& connection, const CallRequest& request) noexcept;

/**
 * Asynchronously call server methods.
 * @copydetails call
 * @param token @completiontoken{void(CallResponse&)}
 * @return @asyncresult{CallResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto callAsync(
    Client& connection,
    const CallRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_CallRequest, UA_CallResponse>(
        connection, request, detail::Wrap<CallResponse>{}, std::forward<CompletionToken>(token)
    );
}

/**
 * Call a server method.
 * The `objectId` must have a `HasComponent` reference to the method specified in `methodId`.
 *
 * @param connection Instance of type Server or Client
 * @param objectId NodeId of the object on which the method is invoked
 * @param methodId NodeId of the method to invoke
 * @param inputArguments Input argument values
 */
template <typename T>
CallMethodResult call(
    T& connection,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) noexcept;

/**
 * Asynchronously call a server method.
 *
 * @param connection Instance of type Client
 * @param objectId NodeId of the object on which the method is invoked
 * @param methodId NodeId of the method to invoke
 * @param inputArguments Input argument values
 * @param token @completiontoken{void(CallMethodResult&)}
 * @return @asyncresult{CallMethodResult}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto callAsync(
    Client& connection,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments,
    CompletionToken&& token = DefaultCompletionToken()
) {
    auto item = detail::createCallMethodRequest(objectId, methodId, inputArguments);
    const auto request = detail::createCallRequest(item);
    return callAsync(
        connection,
        asWrapper<CallRequest>(request),
        detail::TransformToken{
            detail::wrapSingleResultWithStatus<CallMethodResult, UA_CallResponse>,
            std::forward<CompletionToken>(token)
        }
    );
}

/**
 * @}
 * @}
 */

}  // namespace opcua::services

#endif
