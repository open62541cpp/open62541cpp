#include <filesystem>

#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/plugin/nodesetloader.hpp"

#if UAPP_HAS_NODESETLOADER

namespace fs = std::filesystem;
using namespace opcua;

TEST_CASE("loadNodeset") {
    Server server;
    CHECK(server.namespaceArray().size() == 2);

    SUBCASE("Invalid file") {
        CHECK(loadNodeset(server, "invalid.xml").isBad());
    }

    SUBCASE("DI") {
        const auto file = fs::path{UAPP_NODESET_DIR} / "DI" / "Opc.Ua.Di.NodeSet2.xml";
        CAPTURE(file);
        CHECK(fs::exists(file));
        CHECK(loadNodeset(server, file.c_str()).isGood());

        const auto ns = server.namespaceArray();
        CHECK(ns.size() == 3);
        CHECK(ns.back() == "http://opcfoundation.org/UA/DI/");
    }
}

#endif
