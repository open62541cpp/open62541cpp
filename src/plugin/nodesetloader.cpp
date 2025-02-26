#include "open62541pp/plugin/nodesetloader.hpp"

#if UAPP_HAS_NODESETLOADER

#include <string>

#include "open62541pp/detail/open62541/push_options.h"
#include <NodesetLoader/backendOpen62541.h>
#include "open62541pp/detail/open62541/pop_options.h"

#include "open62541pp/server.hpp"
#include "open62541pp/types.hpp"

namespace opcua {

StatusCode loadNodeset(Server& server, std::string_view nodeset2XmlFilePath) {
    const auto success = NodesetLoader_loadFile(
        server.handle(), std::string{nodeset2XmlFilePath}.c_str(), nullptr
    );
    return success ? UA_STATUSCODE_GOOD : UA_STATUSCODE_BAD;
}

}  // namespace opcua

#endif
