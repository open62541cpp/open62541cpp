#include <algorithm>  // any_of

#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/node.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

TEST_CASE_TEMPLATE("Node", T, Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& connection = setup.instance<T>();

    [[maybe_unused]] const auto await = [&](auto future) {
        setup.client.runIterate();
        return future.get();
    };

    const NodeId varId{1, 1};
    REQUIRE(services::addVariable(
        setup.server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        varId,
        "Variable",
        {},
        VariableTypeId::BaseDataVariableType,
        ReferenceTypeId::HasComponent
    ));
    REQUIRE(services::writeAccessLevel(setup.server, varId, 0xFF).isGood());
    REQUIRE(services::writeWriteMask(setup.server, varId, 0xFFFFFFFF).isGood());

    const NodeId objId{1, 2};
    REQUIRE(services::addObject(
        setup.server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        objId,
        "Object",
        {},
        ObjectTypeId::BaseObjectType,
        ReferenceTypeId::HasComponent
    ));
    REQUIRE(services::writeWriteMask(setup.server, objId, 0xFFFFFFFF).isGood());

    const NodeId methodId{1, 3};
    REQUIRE(services::addMethod(
        setup.server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        methodId,
        "Method",
        {},
        {},
        {},
        {},
        ReferenceTypeId::HasComponent
    ));
    REQUIRE(services::writeWriteMask(setup.server, methodId, 0xFFFFFFFF).isGood());

    const NodeId refId{1, 4};
    REQUIRE(services::addReferenceType(
        setup.server,
        {0, UA_NS0ID_REFERENCES},
        refId,
        "ReferenceType",
        {},
        ReferenceTypeId::HasSubtype
    ));
    REQUIRE(services::writeWriteMask(setup.server, refId, 0xFFFFFFFF).isGood());

    Node objNode(connection, objId);
    Node varNode(connection, varId);
    Node methodNode(connection, methodId);
    Node refNode(connection, refId);

    SUBCASE("connection") {
        CHECK(Node(connection, {}).connection() == connection);
    }

    SUBCASE("id") {
        const NodeId id{1, 1000};
        CHECK(Node(connection, id).id() == id);
    }

    SUBCASE("exists") {
        CHECK(Node(connection, NodeId(0, UA_NS0ID_OBJECTSFOLDER)).exists());
        CHECK_FALSE(Node(connection, NodeId(0, "DoesNotExist")).exists());
    }

    SUBCASE("addFolder") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(objNode.addFolderAsync(id, "Folder")).value());
        } else {
            CHECK_NOTHROW(objNode.addFolder(id, "Folder"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Object);
    }

    SUBCASE("addObject") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(objNode.addObjectAsync(id, "Object")).value());
        } else {
            CHECK_NOTHROW(objNode.addObject(id, "Object"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Object);
    }

    SUBCASE("addVariable") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(objNode.addVariableAsync(id, "Variable")).value());
        } else {
            CHECK_NOTHROW(objNode.addVariable(id, "Variable"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Variable);
    }

    SUBCASE("addProperty") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(objNode.addPropertyAsync(id, "Property")).value());
        } else {
            CHECK_NOTHROW(objNode.addProperty(id, "Property"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Variable);
    }

#ifdef UA_ENABLE_METHODCALLS
    SUBCASE("addMethod") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(objNode.addMethodAsync(id, "Method", {}, {}, {})).value());
        } else {
            CHECK_NOTHROW(objNode.addMethod(id, "Method", {}, {}, {}));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Method);
    }
#endif

    SUBCASE("addObjectType") {
        Node parent(connection, ObjectTypeId::BaseObjectType);
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(parent.addObjectTypeAsync(id, "ObjectType")).value());
        } else {
            CHECK_NOTHROW(parent.addObjectType(id, "ObjectType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::ObjectType);
    }

    SUBCASE("addVariableType") {
        Node parent(connection, VariableTypeId::BaseVariableType);
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(parent.addVariableTypeAsync(id, "VariableType")).value());
        } else {
            CHECK_NOTHROW(parent.addVariableType(id, "VariableType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::VariableType);
    }

    SUBCASE("addReferenceType") {
        Node parent(connection, ReferenceTypeId::References);
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(parent.addReferenceTypeAsync(id, "ReferenceType")).value());
        } else {
            CHECK_NOTHROW(parent.addReferenceType(id, "ReferenceType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::ReferenceType);
    }

    SUBCASE("addDataType") {
        Node parent(connection, DataTypeId::BaseDataType);
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(parent.addDataTypeAsync(id, "DataType")).value());
        } else {
            CHECK_NOTHROW(parent.addDataType(id, "DataType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::DataType);
    }

    SUBCASE("addView") {
        Node parent(connection, ObjectId::ViewsFolder);
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(parent.addViewAsync(id, "View")).value());
        } else {
            CHECK_NOTHROW(parent.addView(id, "View"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::View);
    }

    SUBCASE("addReference/deleteReference") {
        auto folder = objNode.addFolder({1, 1000}, "Folder");
        auto object = objNode.addObject({1, 1001}, "Object");
        const NodeId referenceType(ReferenceTypeId::Organizes);

        // add
        if constexpr (isAsync<T>) {
            CHECK(await(folder.addReferenceAsync(object.id(), referenceType)).isGood());
        } else {
            CHECK_NOTHROW(folder.addReference(object.id(), referenceType));
        }

        // delete
        if constexpr (isAsync<T>) {
            CHECK(await(folder.deleteReferenceAsync(object.id(), referenceType)).isGood());
        } else {
            CHECK_NOTHROW(folder.deleteReference(object.id(), referenceType));
        }
    }

    SUBCASE("deleteNode") {
        if constexpr (isAsync<T>) {
            CHECK(await(objNode.deleteNodeAsync()).isGood());
        } else {
            CHECK_NOTHROW(objNode.deleteNode());
        }
    }

    SUBCASE("browseReferences") {
        Node root(connection, ObjectId::RootFolder);
        const auto refs = root.browseReferences();
        CHECK(refs.size() > 0);
        CHECK(std::any_of(refs.begin(), refs.end(), [&](auto& ref) {
            return ref.browseName() == QualifiedName(0, "Objects");
        }));
    }

    SUBCASE("browseReferencedNodes") {
        Node root(connection, ObjectId::RootFolder);
        Node child(connection, ObjectId::ObjectsFolder);
        const auto nodes = child.browseReferencedNodes();
        CHECK(nodes.size() > 0);
        CHECK(std::any_of(nodes.begin(), nodes.end(), [&](auto& node) { return node == root; }));
    }

    SUBCASE("browseChildren") {
        Node root(connection, ObjectId::RootFolder);
        Node child(connection, ObjectId::ObjectsFolder);
        CHECK(root.browseChildren(ReferenceTypeId::HasChild).empty());
        const auto nodes = root.browseChildren(ReferenceTypeId::HierarchicalReferences);
        CHECK(nodes.size() > 0);
        CHECK(std::any_of(nodes.begin(), nodes.end(), [&](auto& node) { return node == child; }));
    }

    SUBCASE("browseChild") {
        Node root(connection, ObjectId::RootFolder);
        CHECK_THROWS_WITH(root.browseChild({{0, "Invalid"}}), "BadNoMatch");
        CHECK_EQ(root.browseChild({{0, "Objects"}}).id(), NodeId(ObjectId::ObjectsFolder));
        CHECK_EQ(root.browseChild({{0, "Objects"}, {0, "Server"}}).id(), NodeId(ObjectId::Server));
    }

    SUBCASE("browseParent") {
        Node root(connection, ObjectId::RootFolder);
        Node child(connection, ObjectId::ObjectsFolder);
        CHECK_THROWS_WITH(root.browseParent(), "BadNotFound");
        CHECK_EQ(child.browseParent(), root);
    }

    SUBCASE("readNodeClass") {
        if constexpr (isAsync<T>) {
            CHECK(await(varNode.readNodeClassAsync()).value() == NodeClass::Variable);
        } else {
            CHECK(varNode.readNodeClass() == NodeClass::Variable);
        }
    }

    SUBCASE("readBrowseName") {
        const QualifiedName browseName(1, "Variable");
        if constexpr (isAsync<T>) {
            CHECK(await(varNode.readBrowseNameAsync()).value() == browseName);
        } else {
            CHECK(varNode.readBrowseName() == browseName);
        }
    }

    SUBCASE("writeDisplayName/readDisplayName") {
        // https://github.com/open62541/open62541/issues/6723
        const LocalizedText displayName({}, "DisplayName");
        if constexpr (isAsync<T>) {
            CHECK(await(varNode.writeDisplayNameAsync(displayName)).isGood());
            CHECK(await(varNode.readDisplayNameAsync()).value() == displayName);
        } else {
            CHECK_NOTHROW(varNode.writeDisplayName(displayName));
            CHECK(varNode.readDisplayName() == displayName);
        }
    }

    SUBCASE("writeDescription/readDescription") {
        const LocalizedText description("en-US", "Description");
        if constexpr (isAsync<T>) {
            CHECK(await(varNode.writeDescriptionAsync(description)).isGood());
            CHECK(await(varNode.readDescriptionAsync()).value() == description);
        } else {
            CHECK_NOTHROW(varNode.writeDescription(description));
            CHECK(varNode.readDescription() == description);
        }
    }

    SUBCASE("writeWriteMask/readWriteMask") {
        const auto writeMask = 0xFFFFFFFF;
        if constexpr (isAsync<T>) {
            CHECK(await(varNode.writeWriteMaskAsync(writeMask)).isGood());
            CHECK(await(varNode.readWriteMaskAsync()).value() == writeMask);
        } else {
            CHECK_NOTHROW(varNode.writeWriteMask(writeMask));
            CHECK(varNode.readWriteMask() == writeMask);
        }
    }

    SUBCASE("readUserWriteMask") {
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(varNode.readUserWriteMaskAsync()).value());
        } else {
            CHECK_NOTHROW(varNode.readUserWriteMask());
        }
    }

    SUBCASE("writeIsAbstract/readIsAbstract") {
        const bool isAbstract = true;
        if constexpr (isAsync<T>) {
            CHECK(await(refNode.writeIsAbstractAsync(isAbstract)).isGood());
            CHECK(await(refNode.readIsAbstractAsync()).value() == isAbstract);
        } else {
            CHECK_NOTHROW(refNode.writeIsAbstract(isAbstract));
            CHECK_NOTHROW(refNode.readIsAbstract());
        }
    }

    SUBCASE("writeSymmetric/readSymmetric") {
        const bool symmetric = true;
        if constexpr (isAsync<T>) {
            CHECK(await(refNode.writeSymmetricAsync(symmetric)).isGood());
            CHECK(await(refNode.readSymmetricAsync()).value() == symmetric);
        } else {
            CHECK_NOTHROW(refNode.writeSymmetric(symmetric));
            CHECK(refNode.readSymmetric() == symmetric);
        }
    }

    SUBCASE("writeInverseName/readInverseName") {
        const LocalizedText inverseName({}, "InverseName");
        if constexpr (isAsync<T>) {
            CHECK(await(refNode.writeInverseNameAsync(inverseName)).isGood());
            CHECK(await(refNode.readInverseNameAsync()).value() == inverseName);
        } else {
            CHECK_NOTHROW(refNode.writeInverseName(inverseName));
            CHECK(refNode.readInverseName() == inverseName);
        }
    }

    SUBCASE("writeEventNotifier/readEventNotifier") {
        const auto eventNotifier = EventNotifier::HistoryRead | EventNotifier::HistoryWrite;
        if constexpr (isAsync<T>) {
            CHECK(await(objNode.writeEventNotifierAsync(eventNotifier)).isGood());
            CHECK(await(objNode.readEventNotifierAsync()).value() == eventNotifier);
        } else {
            CHECK_NOTHROW(objNode.writeEventNotifier(eventNotifier));
            CHECK(objNode.readEventNotifier() == eventNotifier);
        }
    }

    SUBCASE("writeValue/readValue") {
        const double value = 11.11;
        const auto variant = Variant(value);
        if constexpr (isAsync<T>) {
            CHECK(await(varNode.writeValueAsync(variant)).isGood());
            CHECK(await(varNode.readValueAsync()).value().template scalar<double>() == value);
        } else {
            CHECK_NOTHROW(varNode.writeValue(variant));
            CHECK(varNode.readValue().template scalar<double>() == value);
        }
    }

    SUBCASE("writeValue*/readValue*") {
        SUBCASE("Scalar") {
            CHECK_NOTHROW(varNode.writeDataType(DataTypeId::Float));

            // write with wrong data type
            CHECK_THROWS(varNode.writeValueScalar(bool{}));
            CHECK_THROWS(varNode.writeValueScalar(int{}));

            // write with correct data type
            float value = 11.11f;
            CHECK_NOTHROW(varNode.writeValueScalar(value));
            CHECK(varNode.template readValueScalar<float>() == value);
        }

        SUBCASE("String") {
            CHECK_NOTHROW(varNode.writeDataType(DataTypeId::String));

            String str("test");
            CHECK_NOTHROW(varNode.writeValueScalar(str));
            CHECK(varNode.template readValueScalar<std::string>() == "test");
        }

        SUBCASE("Array") {
            CHECK_NOTHROW(varNode.writeDataType(DataTypeId::Double));

            // write with wrong data type
            CHECK_THROWS(varNode.writeValueArray(std::vector<int>{}));
            CHECK_THROWS(varNode.writeValueArray(std::vector<float>{}));

            // write with correct data type
            std::vector<double> array{11.11, 22.22, 33.33};

            SUBCASE("Write as std::vector") {
                CHECK_NOTHROW(varNode.writeValueArray(array));
                CHECK(varNode.template readValueArray<double>() == array);
            }

            SUBCASE("Write as raw array") {
                CHECK_NOTHROW(varNode.writeValueArray(Span{array.data(), array.size()}));
                CHECK(varNode.template readValueArray<double>() == array);
            }

            SUBCASE("Write as iterator pair") {
                CHECK_NOTHROW(varNode.writeValueArray(array.begin(), array.end()));
                CHECK(varNode.template readValueArray<double>() == array);
            }
        }
    }

    SUBCASE("writeDataType/readDataType") {
        const NodeId dataType(DataTypeId::Float);
        if constexpr (isAsync<T>) {
            CHECK(await(varNode.writeDataTypeAsync(dataType)).isGood());
            CHECK(await(varNode.readDataTypeAsync()).value() == dataType);
        } else {
            CHECK_NOTHROW(varNode.writeDataType(dataType));
            CHECK(varNode.readDataType() == dataType);
        }
    }

    SUBCASE("writeValueRank/readValueRank") {
        const auto valueRank = ValueRank::Scalar;
        if constexpr (isAsync<T>) {
            CHECK(await(varNode.writeValueRankAsync(valueRank)).isGood());
            CHECK(await(varNode.readValueRankAsync()).value() == valueRank);
        } else {
            CHECK_NOTHROW(varNode.writeValueRank(valueRank));
            CHECK(varNode.readValueRank() == valueRank);
        }
    }

    SUBCASE("writeArrayDimensions/readArrayDimensions") {
        const std::vector<uint32_t> dimensions{11};
        varNode.writeValueRank(ValueRank::OneDimension);
        if constexpr (isAsync<T>) {
            CHECK(await(varNode.writeArrayDimensionsAsync(dimensions)).isGood());
            CHECK(await(varNode.readArrayDimensionsAsync()).value().at(0) == 11);
        } else {
            CHECK_NOTHROW(varNode.writeArrayDimensions(dimensions));
            CHECK(varNode.readArrayDimensions() == dimensions);
        }
    }

    SUBCASE("readAccessLevel") {
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(varNode.readAccessLevelAsync()).value());
        } else {
            CHECK_NOTHROW(varNode.readAccessLevel());
        }
    }

    SUBCASE("readUserAccessLevel") {
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(varNode.readUserAccessLevelAsync()).value());
        } else {
            CHECK_NOTHROW(varNode.readUserAccessLevel());
        }
    }

    SUBCASE("writeMinimumSamplingInterval/readMinimumSamplingInterval") {
        const double milliseconds = 11.11;
        if constexpr (isAsync<T>) {
            CHECK(await(varNode.writeMinimumSamplingIntervalAsync(milliseconds)).isGood());
            CHECK(await(varNode.readMinimumSamplingIntervalAsync()).value() == milliseconds);
        } else {
            CHECK_NOTHROW(varNode.writeMinimumSamplingInterval(milliseconds));
            CHECK(varNode.readMinimumSamplingInterval() == milliseconds);
        }
    }

    SUBCASE("writeHistorizing/readHistorizing") {
        const bool historizing = true;
        if constexpr (isAsync<T>) {
            CHECK(await(varNode.writeHistorizingAsync(historizing)).isGood());
            CHECK(await(varNode.readHistorizingAsync()).value() == historizing);
        } else {
            CHECK_NOTHROW(varNode.writeHistorizing(historizing));
            CHECK(varNode.readHistorizing() == historizing);
        }
    }

    SUBCASE("writeExecutable/readExecutable") {
        const bool executable = true;
        if constexpr (isAsync<T>) {
            CHECK(await(methodNode.writeExecutableAsync(executable)).isGood());
            CHECK(await(methodNode.readExecutableAsync()).value() == executable);
        } else {
            CHECK_NOTHROW(methodNode.writeExecutable(executable));
            CHECK(methodNode.readExecutable() == executable);
        }
    }

    SUBCASE("readUserExecutable") {
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(methodNode.readUserExecutableAsync()).value());
        } else {
            CHECK_NOTHROW(methodNode.readUserExecutable());
        }
    }

#if UAPP_OPEN62541_VER_GE(1, 1)
    SUBCASE("readDataTypeDefinition") {
        Node node(connection, DataTypeId::BuildInfo);
        if constexpr (isAsync<T>) {
            CHECK_NOTHROW(await(node.readDataTypeDefinitionAsync()).value());
        } else {
            CHECK_NOTHROW(node.readDataTypeDefinition());
        }
    }
#endif

    SUBCASE("writeObjectProperty/readObjectProperty") {
        objNode.addProperty(
            {1, 1001},
            "Property",
            VariableAttributes{}
                .setWriteMask(WriteMask::None)
                .setAccessLevel(AccessLevel::CurrentRead | AccessLevel::CurrentWrite)
                .setDataType<double>()
                .setValueScalar(11.11)
        );
        CHECK(objNode.readObjectProperty({1, "Property"}).template scalar<double>() == 11.11);
        CHECK_NOTHROW(objNode.writeObjectProperty({1, "Property"}, Variant(22.22)));
        CHECK(objNode.readObjectProperty({1, "Property"}).template scalar<double>() == 22.22);
    }

    SUBCASE("Equality") {
        const NodeId id1{1, 1000};
        const NodeId id2{1, 1001};
        CHECK(Node(connection, id1) == Node(connection, id1));
        CHECK(Node(connection, id1) != Node(connection, id2));
    }
}
