#pragma once

#include <cstdint>
#include <functional>  // invoke
#include <iterator>  // make_move_iterator
#include <memory>  // enable_shared_from_this, make_shared
#include <type_traits>
#include <utility>
#include <vector>

#include "open62541pp/async.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/result.hpp"
#include "open62541pp/services/detail/async_transform.hpp"
#include "open62541pp/services/detail/client_service.hpp"
#include "open62541pp/services/detail/request_handling.hpp"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/ua/types.hpp"

namespace opcua {
class Client;
class Server;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup View View service set
 * Browse the address space / view created by a server.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.9
 * @ingroup Services
 * @{
 */

/**
 * @defgroup Browse Browse service
 * Discover references of nodes.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.9.2
 * @{
 */

/**
 * Discover the references of one or more nodes (client only).
 * @param connection Instance of type Client
 * @param request Browse request
 */
BrowseResponse browse(Client& connection, const BrowseRequest& request) noexcept;

/**
 * @copydoc browse
 * @param token @completiontoken{void(BrowseResponse&)}
 * @return @asyncresult{BrowseResponse}
 */
template <typename CompletionToken>
auto browseAsync(Client& connection, const BrowseRequest& request, CompletionToken&& token) {
    return detail::sendRequestAsync<BrowseRequest, BrowseResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * Discover the references of a specified node.
 * @param connection Instance of type Server or Client
 * @param bd Browse description
 * @param maxReferences The maximum number of references to return (0 if no limit)
 */
template <typename T>
BrowseResult browse(T& connection, const BrowseDescription& bd, uint32_t maxReferences) noexcept;

/**
 * @copydoc browse(T&, const BrowseDescription&, uint32_t)
 * @param token @completiontoken{void(BrowseResult&)}
 * @return @asyncresult{BrowseResult}
 */
template <typename CompletionToken>
auto browseAsync(
    Client& connection, const BrowseDescription& bd, uint32_t maxReferences, CompletionToken&& token
) {
    const auto request = detail::createBrowseRequest(bd, maxReferences);
    return browseAsync(
        connection,
        asWrapper<BrowseRequest>(request),
        detail::TransformToken(
            detail::wrapSingleResultWithStatus<BrowseResult, UA_BrowseResponse>,
            std::forward<CompletionToken>(token)
        )
    );
}

/**
 * @}
 * @defgroup BrowseNext BrowseNext service
 * Request the next set of a Browse or BrowseNext response information that is too large to be sent
 * in a single response. Discover references of nodes.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.9.3
 * @{
 */

/**
 * Request the next sets of @ref browse / @ref browseNext responses (client only).
 * @param connection Instance of type Client
 * @param request Browse request
 */
BrowseNextResponse browseNext(Client& connection, const BrowseNextRequest& request) noexcept;

/**
 * @copydoc browseNext
 * @param token @completiontoken{void(BrowseNextResponse&)}
 * @return @asyncresult{BrowseNextResponse}
 */
template <typename CompletionToken>
auto browseNextAsync(
    Client& connection, const BrowseNextRequest& request, CompletionToken&& token
) {
    return detail::sendRequestAsync<BrowseNextRequest, BrowseNextResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * Request the next set of a @ref browse or @ref browseNext response.
 * The response might get split up if the information is too large to be sent in a single response.
 * @param connection Instance of type Server or Client
 * @param releaseContinuationPoint Free resources in server if `true`, get next result if `false`
 * @param continuationPoint Continuation point from a preview browse/browseNext request
 */
template <typename T>
BrowseResult browseNext(
    T& connection, bool releaseContinuationPoint, const ByteString& continuationPoint
) noexcept;

/**
 * @copydoc browseNext(T&, bool, const ByteString&)
 * @param token @completiontoken{void(BrowseResult&)}
 * @return @asyncresult{BrowseResult}
 */
template <typename CompletionToken>
auto browseNextAsync(
    Client& connection,
    bool releaseContinuationPoint,
    const ByteString& continuationPoint,
    CompletionToken&& token
) {
    const auto request = detail::createBrowseNextRequest(
        releaseContinuationPoint, continuationPoint
    );
    return browseNextAsync(
        connection,
        asWrapper<BrowseNextRequest>(request),
        detail::TransformToken(
            detail::wrapSingleResultWithStatus<BrowseResult, UA_BrowseNextResponse>,
            std::forward<CompletionToken>(token)
        )
    );
}

/**
 * @}
 * @defgroup TranslateBrowsePathsToNodeIds TranslateBrowsePathsToNodeIds service
 * Request that the server translates browse paths to node ids.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.9.4
 * @{
 */

/**
 * Translate browse paths to NodeIds (client only).
 * @param connection Instance of type Client
 * @param request Request
 */
TranslateBrowsePathsToNodeIdsResponse translateBrowsePathsToNodeIds(
    Client& connection, const TranslateBrowsePathsToNodeIdsRequest& request
) noexcept;

/**
 * @copydoc translateBrowsePathsToNodeIds
 * @param token @completiontoken{void(TranslateBrowsePathsToNodeIdsResponse&)}
 * @return @asyncresult{TranslateBrowsePathsToNodeIdsResponse}
 */
template <typename CompletionToken>
auto translateBrowsePathsToNodeIdsAsync(
    Client& connection, const TranslateBrowsePathsToNodeIdsRequest& request, CompletionToken&& token
) {
    return detail::sendRequestAsync<
        TranslateBrowsePathsToNodeIdsRequest,
        TranslateBrowsePathsToNodeIdsResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * Translate a browse path to NodeIds.
 * @param connection Instance of type Server or Client
 * @param browsePath Browse path (starting node & relative path)
 */
template <typename T>
BrowsePathResult translateBrowsePathToNodeIds(T& connection, const BrowsePath& browsePath) noexcept;

/**
 * @copydoc translateBrowsePathToNodeIds
 * @param token @completiontoken{void(BrowsePathResult&)}
 * @return @asyncresult{BrowsePathResult}
 */
template <typename CompletionToken>
auto translateBrowsePathToNodeIdsAsync(
    Client& connection, const BrowsePath& browsePath, CompletionToken&& token
) {
    const auto request = detail::createTranslateBrowsePathsToNodeIdsRequest(browsePath);
    return translateBrowsePathsToNodeIdsAsync(
        connection,
        asWrapper<TranslateBrowsePathsToNodeIdsRequest>(request),
        detail::TransformToken(
            detail::wrapSingleResultWithStatus<
                BrowsePathResult,
                UA_TranslateBrowsePathsToNodeIdsResponse>,
            std::forward<CompletionToken>(token)
        )
    );
}

/**
 * A simplified version of @ref translateBrowsePathToNodeIds.
 *
 * The relative path is specified using browse names instead of the RelativePath structure.
 * The list of browse names is equivalent to a RelativePath that specifies forward references which
 * are subtypes of the HierarchicalReferences ReferenceTypeId.
 *
 * @param connection Instance of type Server or Client
 * @param origin Starting node of the browse path
 * @param browsePath Browse path as a list of browse names
 */
template <typename T>
BrowsePathResult browseSimplifiedBrowsePath(
    T& connection, const NodeId& origin, Span<const QualifiedName> browsePath
) {
    return translateBrowsePathToNodeIds(connection, detail::createBrowsePath(origin, browsePath));
}

/**
 * @copydoc browseSimplifiedBrowsePath
 * @param token @completiontoken{void(BrowsePathResult&)}
 * @return @asyncresult{BrowsePathResult}
 */
template <typename CompletionToken>
auto browseSimplifiedBrowsePathAsync(
    Client& connection,
    const NodeId& origin,
    Span<const QualifiedName> browsePath,
    CompletionToken&& token
) {
    return translateBrowsePathToNodeIdsAsync(
        connection,
        detail::createBrowsePath(origin, browsePath),
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 * @defgroup RegisterNodes RegisterNodes service
 * Register nodes for efficient access operations.
 * Clients shall unregister unneeded nodes immediately to free up resources.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.9.5
 * @{
 */

/**
 * Register nodes for efficient access operations (client only).
 * @param connection Instance of type Client
 * @param request Request
 */
RegisterNodesResponse registerNodes(
    Client& connection, const RegisterNodesRequest& request
) noexcept;

/**
 * @copydoc registerNodes
 * @param token @completiontoken{void(RegisterNodesResponse&)}
 * @return @asyncresult{RegisterNodesResponse}
 */
template <typename CompletionToken>
auto registerNodesAsync(
    Client& connection, const RegisterNodesRequest& request, CompletionToken&& token
) {
    return detail::sendRequestAsync<RegisterNodesRequest, RegisterNodesResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 * @defgroup UnregisterNodes UnregisterNodes service
 * Unregister nodes that have been obtained via the RegisterNodes service.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.9.6
 * @{
 */

/**
 * Unregister nodes (client only).
 * @param connection Instance of type Client
 * @param request Request
 */
UnregisterNodesResponse unregisterNodes(
    Client& connection, const UnregisterNodesRequest& request
) noexcept;

/**
 * @copydoc unregisterNodes
 * @param token @completiontoken{void(UnregisterNodesResponse&)}
 * @return @asyncresult{UnregisterNodesResponse}
 */
template <typename CompletionToken>
auto unregisterNodesAsync(
    Client& connection, const UnregisterNodesRequest& request, CompletionToken&& token
) {
    return detail::sendRequestAsync<UnregisterNodesRequest, UnregisterNodesResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 */

/* ----------------------------------- Non-standard functions ----------------------------------- */

// default maxReferences arg of initial browse request within browseAll, primarily for testing
#ifndef UAPP_BROWSE_ALL_MAX_REFERENCES
#define UAPP_BROWSE_ALL_MAX_REFERENCES 0U  // NOLINT(*macro-usage)
#endif

/**
 * Discover all the references of a specified node (without calling @ref browseNext).
 * @param connection Instance of type Server or Client
 * @param bd Browse description
 * @ingroup Browse
 */
template <typename T>
Result<std::vector<ReferenceDescription>> browseAll(T& connection, const BrowseDescription& bd) {
    std::vector<ReferenceDescription> refs;
    auto handler = [&](BrowseResult& result) {
        refs.insert(
            refs.end(),
            std::make_move_iterator(result.references().begin()),
            std::make_move_iterator(result.references().end())
        );
    };
    BrowseResult result = browse(connection, bd, UAPP_BROWSE_ALL_MAX_REFERENCES);
    handler(result);
    while (!result.continuationPoint().empty()) {
        result = browseNext(connection, false, result.continuationPoint());
        handler(result);
    }
    return {std::move(refs), result.statusCode()};
}

/**
 * @copydoc browseAll
 * @param token @completiontoken{void(Result<std::vector<ReferenceDescription>>&)}
 * @return @asyncresult{Result<std::vector<ReferenceDescription>>}
 * @ingroup Browse
 */
template <typename CompletionToken>
auto browseAllAsync(Client& connection, const BrowseDescription& bd, CompletionToken&& token) {
    return asyncInitiate<Result<std::vector<ReferenceDescription>>>(
        [&](auto&& handler) {
            using HandlerType = std::remove_reference_t<decltype(handler)>;
            struct RecursiveHandler : std::enable_shared_from_this<RecursiveHandler> {
                RecursiveHandler(Client& innerClient, HandlerType innerHandler)
                    : client(innerClient),
                      handler(std::move(innerHandler)) {}

                void handle(BrowseResult& result) {
                    refs.insert(
                        refs.end(),
                        std::make_move_iterator(result.getReferences().begin()),
                        std::make_move_iterator(result.getReferences().end())
                    );
                    if (result.getContinuationPoint().empty()) {
                        Result refsResult(std::move(refs), result.getStatusCode());
                        std::invoke(handler, refsResult);
                    } else {
                        browseNextAsync(
                            client,
                            false,
                            result.getContinuationPoint(),
                            [self = this->shared_from_this()](BrowseResult& innerResult) {
                                return self->handle(innerResult);
                            }
                        );
                    }
                }

                Client& client;  // NOLINT(*ref-data-members)
                HandlerType handler;
                std::vector<ReferenceDescription> refs;
            };

            auto handlerPtr = std::make_shared<RecursiveHandler>(
                connection, std::forward<decltype(handler)>(handler)
            );
            browseAsync(
                connection,
                bd,
                UAPP_BROWSE_ALL_MAX_REFERENCES,
                [handlerPtr = std::move(handlerPtr)](BrowseResult& result) {
                    handlerPtr->handle(result);
                }
            );
        },
        std::forward<CompletionToken>(token)
    );
}

/**
 * Discover child nodes recursively (non-standard).
 *
 * Possible loops (that can occur for non-hierarchical references) are handled internally. Every
 * node is added at most once to the results array. Nodes are only added if they match the
 * `nodeClassMask` in the BrowseDescription. However, child nodes are still recursed into if the
 * NodeClass does not match. So it is possible, for example, to get all VariableNodes below a
 * certain ObjectNode, with additional objects in the hierarchy below.
 *
 * @note No implementation for `Client`.
 *
 * @param connection Instance of type Server
 * @param bd Browse description
 * @ingroup Browse
 * @see UA_Server_browseRecursive
 */
[[deprecated("will be removed in the future, use UA_Server_browseRecursive instead")]]
Result<std::vector<ExpandedNodeId>> browseRecursive(
    Server& connection, const BrowseDescription& bd
);

/**
 * @}
 */

}  // namespace opcua::services
