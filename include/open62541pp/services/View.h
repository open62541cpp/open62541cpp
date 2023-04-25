#pragma once

#include <iterator>
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/NodeId.h"

// forward declarations
namespace opcua {
class Server;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup View View service set
 * Browse the address space / view created by a server.
 * @ingroup Services
 */

/**
 * Discover the references of a specified node.
 * @param serverOrClient Instance of type Server or Client
 * @param bd Browse description
 * @param maxReferences The maximum number of references to return (0 if no limit)
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.2
 * @ingroup View
 */
template <typename T>
BrowseResult browse(T& serverOrClient, const BrowseDescription& bd, uint32_t maxReferences = 0);

/**
 * Request the next set of a @ref browse or @ref browseNext response.
 * The response might get split up if the information is too large to be sent in a single response.
 * @param serverOrClient Instance of type Server or Client
 * @param releaseContinuationPoint Free resources in server if `true`, get next result if `false`
 * @param continuationPoint Continuation point from a preview browse/browseNext request
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.3
 * @ingroup View
 */
template <typename T>
BrowseResult browseNext(
    T& serverOrClient, bool releaseContinuationPoint, const ByteString& continuationPoint
);

/**
 * Discover all the references of a specified node (without calling @ref browseNext).
 * @copydetails browse
 * @ingroup View
 */
template <typename T>
std::vector<ReferenceDescription> browseAll(
    T& serverOrClient, const BrowseDescription& bd, uint32_t maxReferences = 0
);

/**
 * Translate a browse path to NodeIds.
 * @param serverOrClient Instance of type Server or Client
 * @param browsePath Browse path (starting node & relative path)
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.4
 * @ingroup View
 */
template <typename T>
BrowsePathResult translateBrowsePathToNodeIds(T& serverOrClient, const BrowsePath& browsePath);

/**
 * A simplified version of @ref translateBrowsePathToNodeIds.
 *
 * The relative path is specified using browse names instead of the RelativePath structure.
 * The list of browse names is equivalent to a RelativePath that specifies forward references which
 * are subtypes of the HierarchicalReferences ReferenceType.
 *
 * @param serverOrClient Instance of type Server or Client
 * @param origin Starting node of the browse path
 * @param browsePath Browse path as a list of browse names
 * @ingroup View
 */
template <typename T>
BrowsePathResult browseSimplifiedBrowsePath(
    T& serverOrClient, const NodeId& origin, const std::vector<QualifiedName>& browsePath
);

/**
 * Get a child specified by its path from this node (only local nodes).
 * @exception BadStatus If path not found (BadNoMatch)
 * @ingroup View
 */
NodeId browseChild(Server& server, const NodeId& origin, const std::vector<QualifiedName>& path);

}  // namespace opcua::services
