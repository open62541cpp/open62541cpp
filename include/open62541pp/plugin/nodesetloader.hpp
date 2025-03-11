#pragma once

#include <string_view>

#include "open62541pp/config.hpp"

#if UAPP_HAS_NODESETLOADER

namespace opcua {

class Server;
class StatusCode;

/**
 * Load a nodeset from a NodeSet2.xml file at runtime.
 * @param server The server to load the nodeset into.
 * @param nodeset2XmlFilePath The path to the XML file.
 */
StatusCode loadNodeset(Server& server, std::string_view nodeset2XmlFilePath);

}  // namespace opcua

#endif
