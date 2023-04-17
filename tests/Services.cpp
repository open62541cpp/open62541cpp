#include <functional>  // reference_wrapper
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"
#include "open62541pp/services/services.h"

#include "helper/Runner.h"

using namespace Catch::Matchers;
using namespace opcua;

TEST_CASE("NodeManagement (server)") {
    Server server;
    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SECTION("Non-type nodes") {
        services::addFolder(server, objectsId, {1, 1000}, "folder");
        services::addObject(server, objectsId, {1, 1001}, "object");
        services::addVariable(server, objectsId, {1, 1002}, "variable");
        services::addProperty(server, objectsId, {1, 1003}, "property");
    }

    SECTION("Type nodes") {
        services::addObjectType(server, {0, UA_NS0ID_BASEOBJECTTYPE}, {1, 1000}, "objecttype");
        services::addVariableType(
            server, {0, UA_NS0ID_BASEVARIABLETYPE}, {1, 1001}, "variabletype"
        );
    }

    SECTION("Add existing node id") {
        services::addObject(server, objectsId, {1, 1000}, "object1");
        REQUIRE_THROWS_WITH(
            services::addObject(server, objectsId, {1, 1000}, "object2"), "BadNodeIdExists"
        );
    }

    SECTION("Add reference") {
        services::addFolder(server, objectsId, {1, 1000}, "folder");
        services::addObject(server, objectsId, {1, 1001}, "object");
        services::addReference(server, {1, 1000}, {1, 1001}, ReferenceType::Organizes);
        REQUIRE_THROWS_WITH(
            services::addReference(server, {1, 1000}, {1, 1001}, ReferenceType::Organizes),
            "BadDuplicateReferenceNotAllowed"
        );
    }

    SECTION("Delete node") {
        services::addObject(server, objectsId, {1, 1000}, "object");
        services::deleteNode(server, {1, 1000});
        REQUIRE_THROWS_WITH(services::deleteNode(server, {1, 1000}), "BadNodeIdUnknown");
    }
}

TEST_CASE("NodeManagement (client)") {
    Server server;
    ServerRunner serverRunner(server);

    Client client;
    client.connect("opc.tcp://localhost:4840");

    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SECTION("Non-type nodes") {
        REQUIRE_NOTHROW(services::addObject(client, objectsId, {1, 1000}, "object"));
        REQUIRE(services::readNodeClass(server, {1, 1000}) == NodeClass::Object);

        REQUIRE_NOTHROW(services::addFolder(client, objectsId, {1, 1001}, "folder"));
        REQUIRE(services::readNodeClass(server, {1, 1001}) == NodeClass::Object);

        REQUIRE_NOTHROW(services::addVariable(client, objectsId, {1, 1002}, "variable"));
        REQUIRE(services::readNodeClass(server, {1, 1002}) == NodeClass::Variable);

        REQUIRE_NOTHROW(services::addProperty(client, objectsId, {1, 1003}, "property"));
        REQUIRE(services::readNodeClass(server, {1, 1003}) == NodeClass::Variable);
    }

    SECTION("Type nodes") {
        REQUIRE_NOTHROW(
            services::addObjectType(client, {0, UA_NS0ID_BASEOBJECTTYPE}, {1, 1000}, "objecttype")
        );
        REQUIRE(services::readNodeClass(server, {1, 1000}) == NodeClass::ObjectType);

        REQUIRE_NOTHROW(services::addVariableType(
            client, {0, UA_NS0ID_BASEVARIABLETYPE}, {1, 1001}, "variabletype"
        ));
        REQUIRE(services::readNodeClass(server, {1, 1001}) == NodeClass::VariableType);
    }

    SECTION("Add reference") {
        services::addFolder(client, objectsId, {1, 1000}, "folder");
        services::addObject(client, objectsId, {1, 1001}, "object");
        services::addReference(client, {1, 1000}, {1, 1001}, ReferenceType::Organizes);
        REQUIRE_THROWS_WITH(
            services::addReference(client, {1, 1000}, {1, 1001}, ReferenceType::Organizes),
            "BadDuplicateReferenceNotAllowed"
        );
    }

    SECTION("Delete node") {
        services::addObject(client, objectsId, {1, 1000}, "object");
        services::deleteNode(client, {1, 1000});
        REQUIRE_THROWS_WITH(services::deleteNode(client, {1, 1000}), "BadNodeIdUnknown");
    }
}

TEST_CASE("Attribute (server)") {
    Server server;
    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SECTION("Read/write node attributes") {
        const NodeId id{1, "testAttributes"};
        services::addVariable(server, objectsId, id, "testAttributes");

        // read default attributes
        REQUIRE(services::readNodeId(server, id) == id);
        REQUIRE(services::readNodeClass(server, id) == NodeClass::Variable);
        REQUIRE(services::readBrowseName(server, id) == "testAttributes");
        // REQUIRE(services::readDisplayName(server, id) == LocalizedText("", "testAttributes"));
        REQUIRE(services::readDescription(server, id).getText().empty());
        REQUIRE(services::readDescription(server, id).getLocale().empty());
        REQUIRE(services::readWriteMask(server, id) == 0);
        const uint32_t adminUserWriteMask = ~0;  // all bits set
        REQUIRE(services::readUserWriteMask(server, id) == adminUserWriteMask);
        REQUIRE(services::readDataType(server, id) == NodeId(0, UA_NS0ID_BASEDATATYPE));
        REQUIRE(services::readValueRank(server, id) == ValueRank::Any);
        REQUIRE(services::readArrayDimensions(server, id).empty());
        REQUIRE(services::readAccessLevel(server, id) == UA_ACCESSLEVELMASK_READ);
        const uint8_t adminUserAccessLevel = ~0;  // all bits set
        REQUIRE(services::readUserAccessLevel(server, id) == adminUserAccessLevel);
        REQUIRE(services::readMinimumSamplingInterval(server, id) == 0.0);

        // write new attributes
        REQUIRE_NOTHROW(services::writeDisplayName(server, id, {"en-US", "newDisplayName"}));
        REQUIRE_NOTHROW(services::writeDescription(server, id, {"de-DE", "newDescription"}));
        REQUIRE_NOTHROW(services::writeWriteMask(server, id, 11));
        REQUIRE_NOTHROW(services::writeDataType(server, id, NodeId{0, 2}));
        REQUIRE_NOTHROW(services::writeValueRank(server, id, ValueRank::TwoDimensions));
        REQUIRE_NOTHROW(services::writeArrayDimensions(server, id, {3, 2}));
        const uint8_t newAccessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        REQUIRE_NOTHROW(services::writeAccessLevel(server, id, newAccessLevel));
        REQUIRE_NOTHROW(services::writeMinimumSamplingInterval(server, id, 10.0));

        // read new attributes
        REQUIRE(services::readDisplayName(server, id) == LocalizedText("en-US", "newDisplayName"));
        REQUIRE(services::readDescription(server, id) == LocalizedText("de-DE", "newDescription"));
        REQUIRE(services::readWriteMask(server, id) == 11);
        REQUIRE(services::readDataType(server, id) == NodeId(0, 2));
        REQUIRE(services::readValueRank(server, id) == ValueRank::TwoDimensions);
        REQUIRE(services::readArrayDimensions(server, id).size() == 2);
        REQUIRE(services::readArrayDimensions(server, id).at(0) == 3);
        REQUIRE(services::readArrayDimensions(server, id).at(1) == 2);
        REQUIRE(services::readAccessLevel(server, id) == newAccessLevel);
        REQUIRE(services::readMinimumSamplingInterval(server, id) == 10.0);
    }

    SECTION("Read/write reference node attributes") {
        const NodeId id{0, UA_NS0ID_REFERENCES};

        // read default attributes
        REQUIRE(services::readIsAbstract(server, id) == true);
        REQUIRE(services::readSymmetric(server, id) == true);
        REQUIRE(services::readInverseName(server, id) == LocalizedText("", "References"));

        // write new attributes
        REQUIRE_NOTHROW(services::writeIsAbstract(server, id, false));
        REQUIRE_NOTHROW(services::writeSymmetric(server, id, false));
        REQUIRE_NOTHROW(services::writeInverseName(server, id, LocalizedText("", "New")));

        // read new attributes
        REQUIRE(services::readIsAbstract(server, id) == false);
        REQUIRE(services::readSymmetric(server, id) == false);
        REQUIRE(services::readInverseName(server, id) == LocalizedText("", "New"));
    }

    SECTION("Value rank and array dimension combinations") {
        const NodeId id{1, "testDimensions"};
        services::addVariable(server, objectsId, id, "testDimensions");

        SECTION("Unspecified dimension (ValueRank <= 0)") {
            auto valueRank = GENERATE(
                ValueRank::Any,
                ValueRank::Scalar,
                ValueRank::ScalarOrOneDimension,
                ValueRank::OneOrMoreDimensions
            );
            CAPTURE(valueRank);
            services::writeValueRank(server, id, valueRank);
            CHECK_NOTHROW(services::writeArrayDimensions(server, id, {}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2, 3}));
        }

        SECTION("OneDimension") {
            services::writeValueRank(server, id, ValueRank::OneDimension);
            services::writeArrayDimensions(server, id, {1});
            CHECK_THROWS(services::writeArrayDimensions(server, id, {}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2, 3}));
        }

        SECTION("TwoDimensions") {
            services::writeValueRank(server, id, ValueRank::TwoDimensions);
            services::writeArrayDimensions(server, id, {1, 2});
            CHECK_THROWS(services::writeArrayDimensions(server, id, {}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2, 3}));
        }

        SECTION("ThreeDimensions") {
            services::writeValueRank(server, id, ValueRank::ThreeDimensions);
            services::writeArrayDimensions(server, id, {1, 2, 3});
            CHECK_THROWS(services::writeArrayDimensions(server, id, {}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2}));
        }
    }

    SECTION("Read/write value") {
        const NodeId id{1, "testValue"};
        services::addVariable(server, objectsId, id, "testValue");

        Variant variantWrite;
        variantWrite.setScalarCopy(11.11);
        services::writeValue(server, id, variantWrite);

        Variant variantRead;
        services::readValue(server, id, variantRead);
        CHECK(variantRead.getScalar<double>() == 11.11);
    }

    SECTION("Read/write data value") {
        const NodeId id{1, "testDataValue"};
        services::addVariable(server, objectsId, id, "testDataValue");

        Variant variant;
        variant.setScalarCopy<int>(11);
        DataValue valueWrite(variant, {}, DateTime::now(), {}, 1, UA_STATUSCODE_GOOD);
        services::writeDataValue(server, id, valueWrite);

        DataValue valueRead;
        services::readDataValue(server, id, valueRead);

        CHECK(valueRead->hasValue);
        CHECK(valueRead->hasServerTimestamp);
        CHECK(valueRead->hasSourceTimestamp);
        CHECK(valueRead->hasServerPicoseconds);
        CHECK(valueRead->hasSourcePicoseconds);
        CHECK_FALSE(valueRead->hasStatus);  // doesn't contain error code on success

        CHECK(valueRead.getValue().value().getScalar<int>() == 11);
        CHECK(valueRead->sourceTimestamp == valueWrite->sourceTimestamp);
        CHECK(valueRead->sourcePicoseconds == valueWrite->sourcePicoseconds);
    }
}

TEST_CASE("Attribute (server & client)") {
    Server server;
    ServerRunner serverRunner(server);

    Client client;
    client.connect("opc.tcp://localhost:4840");

    // create variable node
    const NodeId id{1, 1000};
    services::addVariable(server, {0, UA_NS0ID_OBJECTSFOLDER}, id, "variable");
    services::writeAccessLevel(server, id, UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE);
    services::writeWriteMask(server, id, ~0U);  // set all bits to 1 -> allow all

    // check read-only attributes
    CHECK(services::readNodeId(server, id) == services::readNodeId(client, id));
    CHECK(services::readNodeClass(server, id) == services::readNodeClass(client, id));
    CHECK(services::readBrowseName(server, id) == services::readBrowseName(client, id));

    // generate test matrix of possible reader/writer combinations
    // 1. writer: server, reader: server
    // 2. writer: server, reader: client
    // 3. writer: client, reader: server
    // 4. writer: client, reader: client
    using ServerRef = std::reference_wrapper<Server>;
    using ClientRef = std::reference_wrapper<Client>;
    using ServerOrClientRef = std::variant<ServerRef, ClientRef>;

    auto writerVar = GENERATE_REF(as<ServerOrClientRef>{}, std::ref(server), std::ref(client));
    auto readerVar = GENERATE_REF(as<ServerOrClientRef>{}, std::ref(server), std::ref(client));

    CAPTURE(std::holds_alternative<ServerRef>(writerVar));
    CAPTURE(std::holds_alternative<ClientRef>(writerVar));
    CAPTURE(std::holds_alternative<ServerRef>(readerVar));
    CAPTURE(std::holds_alternative<ClientRef>(readerVar));

    std::visit(
        [&](auto writerRef, auto readerRef) {
            auto& writer = writerRef.get();
            auto& reader = readerRef.get();

            const LocalizedText displayName("", "display name");
            CHECK_NOTHROW(services::writeDisplayName(writer, id, displayName));
            CHECK(services::readDisplayName(reader, id) == displayName);

            const LocalizedText description("en-US", "description...");
            CHECK_NOTHROW(services::writeDescription(writer, id, description));
            CHECK(services::readDescription(reader, id) == description);

            const NodeId dataType(0, UA_NS0ID_DOUBLE);
            CHECK_NOTHROW(services::writeDataType(writer, id, dataType));
            CHECK(services::readDataType(reader, id) == dataType);

            const ValueRank valueRank = ValueRank::OneDimension;
            CHECK_NOTHROW(services::writeValueRank(writer, id, valueRank));
            CHECK(services::readValueRank(reader, id) == valueRank);

            std::vector<uint32_t> arrayDimensions{3};
            CHECK_NOTHROW(services::writeArrayDimensions(writer, id, arrayDimensions));
            CHECK(services::readArrayDimensions(reader, id) == arrayDimensions);

            const std::vector<double> array{1, 2, 3};
            const auto variant = Variant::fromArray(array);
            CHECK_NOTHROW(services::writeValue(writer, id, variant));
            Variant variantRead;
            CHECK_NOTHROW(services::readValue(reader, id, variantRead));
            CHECK_THAT(variantRead.getArrayCopy<double>(), Equals(array));

            const auto dataValue = DataValue::fromArray(array);
            CHECK_NOTHROW(services::writeDataValue(writer, id, dataValue));
            DataValue dataValueRead;
            CHECK_NOTHROW(services::readDataValue(reader, id, dataValueRead));
            CHECK(dataValueRead->hasValue);
            CHECK(dataValueRead->hasSourceTimestamp);
            CHECK(dataValueRead->hasServerTimestamp);
            CHECK_THAT(dataValueRead.getValuePtr()->getArrayCopy<double>(), Equals(array));
        },
        writerVar,
        readerVar
    );
}

TEST_CASE("Browse") {
    Server server;

    SECTION("Browse child") {
        const NodeId rootId{0, UA_NS0ID_ROOTFOLDER};

        REQUIRE_THROWS(services::browseChild(server, rootId, {}));
        REQUIRE_THROWS(services::browseChild(server, rootId, {{0, "Invalid"}}));
        REQUIRE(
            services::browseChild(server, rootId, {{0, "Types"}, {0, "ObjectTypes"}}) ==
            NodeId{0, UA_NS0ID_OBJECTTYPESFOLDER}
        );
    }
}
