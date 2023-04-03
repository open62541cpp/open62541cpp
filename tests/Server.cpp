#include <chrono>
#include <thread>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "open62541pp/Helper.h"
#include "open62541pp/Node.h"
#include "open62541pp/Server.h"
#include "open62541pp/types/NodeId.h"

#include "open62541_impl.h"

using namespace Catch::Matchers;
using namespace std::chrono_literals;
using namespace opcua;

TEST_CASE("Server") {
    SECTION("Constructors") {
        SECTION("Default") {
            Server server;
        }
        SECTION("Custom port") {
            Server server(4850);
        }
        SECTION("Custom port and certificate") {
            Server server(4850, "certificate...");
        }
    }

    Server server;

    SECTION("Start / stop server") {
        REQUIRE_FALSE(server.isRunning());

        auto t = std::thread([&] { server.run(); });

        std::this_thread::sleep_for(100ms);  // wait for thread to execute run method

        REQUIRE(server.isRunning());
        REQUIRE_NOTHROW(server.stop());

        t.join();  // wait until stopped
    }

    SECTION("Set hostname / application name / uris") {
        auto* config = UA_Server_getConfig(server.handle());

        server.setCustomHostname("customhost");
        REQUIRE_THAT(detail::toString(config->customHostname), Equals("customhost"));

        server.setApplicationName("Test App");
        REQUIRE_THAT(
            detail::toString(config->applicationDescription.applicationName.text),
            Equals("Test App")
        );

        server.setApplicationUri("http://app.com");
        REQUIRE_THAT(
            detail::toString(config->applicationDescription.applicationUri),
            Equals("http://app.com")
        );

        server.setProductUri("http://product.com");
        REQUIRE_THAT(
            detail::toString(config->applicationDescription.productUri),
            Equals("http://product.com")
        );
    }

    SECTION("Get default nodes") {
        // clang-format off
        CHECK(server.getRootNode().getNodeId()                 == NodeId{0, UA_NS0ID_ROOTFOLDER});
        CHECK(server.getObjectsNode().getNodeId()              == NodeId{0, UA_NS0ID_OBJECTSFOLDER});
        CHECK(server.getTypesNode().getNodeId()                == NodeId{0, UA_NS0ID_TYPESFOLDER});
        CHECK(server.getViewsNode().getNodeId()                == NodeId{0, UA_NS0ID_VIEWSFOLDER});
        CHECK(server.getObjectTypesNode().getNodeId()          == NodeId{0, UA_NS0ID_OBJECTTYPESFOLDER});
        CHECK(server.getVariableTypesNode().getNodeId()        == NodeId{0, UA_NS0ID_VARIABLETYPESFOLDER});
        CHECK(server.getDataTypesNode().getNodeId()            == NodeId{0, UA_NS0ID_DATATYPESFOLDER});
        CHECK(server.getReferenceTypesNode().getNodeId()       == NodeId{0, UA_NS0ID_REFERENCETYPESFOLDER});
        CHECK(server.getBaseObjectTypeNode().getNodeId()       == NodeId{0, UA_NS0ID_BASEOBJECTTYPE});
        CHECK(server.getBaseDataVariableTypeNode().getNodeId() == NodeId{0, UA_NS0ID_BASEDATAVARIABLETYPE});
        // clang-format on
    }

    SECTION("Register namespace") {
        // namespace 0 reserved, but why starting at 2?
        REQUIRE(server.registerNamespace("testnamespaceuri1") == 2);
        REQUIRE(server.registerNamespace("testnamespaceuri2") == 3);
        REQUIRE(server.registerNamespace("testnamespaceuri3") == 4);
    }

    SECTION("Equality") {
        REQUIRE(server == server);
        REQUIRE(server != Server{});
    }
}
