#include <array>
#include <utility>  // move
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "open62541pp/Common.h"
#include "open62541pp/Helper.h"  // detail::toString
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

using namespace Catch::Matchers;
using namespace opcua;

TEMPLATE_TEST_CASE("StringLike", "", String, ByteString, XmlElement) {
    SECTION("Construct with const char*") {
        TestType wrapper("test");
        REQUIRE(wrapper.handle()->length == 4);
        REQUIRE_THAT(std::string(wrapper.get()), Equals("test"));
    }

    SECTION("Construct from non-null-terminated view") {
        std::string str("test123");
        std::string_view sv(str.c_str(), 4);
        TestType wrapper(sv);
        REQUIRE_THAT(std::string(wrapper.get()), Equals("test"));
    }

    SECTION("Equality") {
        REQUIRE(TestType("test") == TestType("test"));
        REQUIRE(TestType("test") != TestType());
    }
}

TEST_CASE("Guid") {
    SECTION("Construct") {
        UA_UInt32 data1{11};
        UA_UInt16 data2{22};
        UA_UInt16 data3{33};
        std::array<UA_Byte, 8> data4{1, 2, 3, 4, 5, 6, 7, 8};

        const Guid wrapper(data1, data2, data3, data4);

        REQUIRE(wrapper.handle()->data1 == data1);
        REQUIRE(wrapper.handle()->data2 == data2);
        REQUIRE(wrapper.handle()->data3 == data3);
        for (int i = 0; i < 8; ++i) {
            REQUIRE(wrapper.handle()->data4[i] == data4[i]);  // NOLINT
        }
    }

    SECTION("Random") {
        REQUIRE(Guid::random() != Guid());
        REQUIRE(Guid::random() != Guid::random());
    }

    SECTION("toString") {
        {
            const Guid guid{};
            REQUIRE_THAT(guid.toString(), Equals("00000000-0000-0000-0000-000000000000"));
        }
        {
            const Guid guid{3298187146, 3582, 19343, {135, 10, 116, 82, 56, 198, 174, 174}};
            REQUIRE_THAT(guid.toString(), Equals("C496578A-0DFE-4B8F-870A-745238C6AEAE"));
        }
    }
}

TEST_CASE("DateTime") {
    SECTION("Empty") {
        const DateTime dt;
        REQUIRE(dt.get() == 0);
        REQUIRE(*dt.handle() == 0);
        // UA time starts before Unix time -> 0
        REQUIRE(dt.toTimePoint().time_since_epoch().count() == 0);
        REQUIRE(dt.toUnixTime() == 0);

        const auto dts = dt.toStruct();
        REQUIRE(dts.nanoSec == 0);
        REQUIRE(dts.microSec == 0);
        REQUIRE(dts.milliSec == 0);
        REQUIRE(dts.sec == 0);
        REQUIRE(dts.min == 0);
        REQUIRE(dts.hour == 0);
        REQUIRE(dts.day == 1);
        REQUIRE(dts.month == 1);
        REQUIRE(dts.year == 1601);
    }

    SECTION("Static methods") {
        REQUIRE_NOTHROW(DateTime::localTimeUtcOffset());
    }

    SECTION("From std::chrono::time_point") {
        using namespace std::chrono;

        const auto now = system_clock::now();
        const int64_t secSinceEpoch = duration_cast<seconds>(now.time_since_epoch()).count();
        const int64_t nsecSinceEpoch = duration_cast<nanoseconds>(now.time_since_epoch()).count();

        const DateTime dt(now);
        REQUIRE(dt.get() == (nsecSinceEpoch / 100) + UA_DATETIME_UNIX_EPOCH);
        REQUIRE(dt.toUnixTime() == secSinceEpoch);
    }

    SECTION("Comparison") {
        const auto zero = DateTime(0);
        const auto now = DateTime::now();
        REQUIRE(zero != now);
        REQUIRE(zero < now);
    }
}

TEST_CASE("NodeId") {
    SECTION("Copy") {
        NodeId src(1, 0);
        NodeId dst(src);
        REQUIRE(dst == src);
    }

    SECTION("Assignment") {
        NodeId src(1, 0);
        NodeId dst(2, 1);
        dst = src;
        REQUIRE(dst == src);
    }

    SECTION("Namespace index") {
        REQUIRE(NodeId(2, 1).getNamespaceIndex() == 2);
        REQUIRE(NodeId(0, 1).getNamespaceIndex() == 0);
    }

    SECTION("Comparison") {
        REQUIRE(NodeId(0, 1) == NodeId(0, 1));
        REQUIRE(NodeId(0, 1) <= NodeId(0, 1));
        REQUIRE(NodeId(0, 1) >= NodeId(0, 1));
        REQUIRE(NodeId(0, 1) != NodeId(0, 2));
        REQUIRE(NodeId(0, 1) != NodeId(1, 1));
        REQUIRE(NodeId(0, "a") == NodeId(0, "a"));
        REQUIRE(NodeId(0, "a") != NodeId(0, "b"));
        REQUIRE(NodeId(0, "a") != NodeId(1, "a"));

        // namespace index is compared before identifier
        REQUIRE(NodeId(0, 1) < NodeId(1, 0));
        REQUIRE(NodeId(0, 1) < NodeId(0, 2));

        REQUIRE(NodeId(1, "a") < NodeId(1, "b"));
        REQUIRE(NodeId(1, "b") > NodeId(1, "a"));
    }

    SECTION("Hash") {
        REQUIRE(NodeId(0, 1).hash() == NodeId(0, 1).hash());
        REQUIRE(NodeId(0, 1).hash() != NodeId(0, 2).hash());
        REQUIRE(NodeId(0, 1).hash() != NodeId(1, 1).hash());
    }

    SECTION("Get properties (getIdentifierType, getNamespaceIndex, getIdentifier") {
        {
            NodeId id(UA_NODEID_NUMERIC(1, 111));
            REQUIRE(id.getIdentifierType() == NodeIdType::Numeric);
            REQUIRE(id.getNamespaceIndex() == 1);
            REQUIRE(id.getIdentifierAs<NodeIdType::Numeric>() == 111);
        }
        {
            NodeId id(UA_NODEID_STRING_ALLOC(2, "Test123"));
            REQUIRE(id.getIdentifierType() == NodeIdType::String);
            REQUIRE(id.getNamespaceIndex() == 2);
            REQUIRE(id.getIdentifierAs<String>() == String("Test123"));
        }
        {
            Guid guid(11, 22, 33, {1, 2, 3, 4, 5, 6, 7, 8});
            NodeId id(3, guid);
            REQUIRE(id.getIdentifierType() == NodeIdType::Guid);
            REQUIRE(id.getNamespaceIndex() == 3);
            REQUIRE(id.getIdentifierAs<Guid>() == guid);
        }
        {
            ByteString byteString("Test456");
            NodeId id(4, byteString);
            REQUIRE(id.getIdentifierType() == NodeIdType::ByteString);
            REQUIRE(id.getNamespaceIndex() == 4);
            REQUIRE(id.getIdentifierAs<ByteString>() == byteString);
        }
    }
}

TEST_CASE("ExpandedNodeId") {
    ExpandedNodeId idLocal({1, "local"}, {}, 0);
    REQUIRE(idLocal.isLocal());
    REQUIRE(idLocal.getNodeId() == NodeId{1, "local"});
    REQUIRE(idLocal.getNodeId().handle() == &idLocal.handle()->nodeId);  // return ref
    REQUIRE(idLocal.getNamespaceUri().empty());
    REQUIRE(idLocal.getServerIndex() == 0);

    ExpandedNodeId idFull({1, "full"}, "namespace", 1);
    REQUIRE(idFull.getNodeId() == NodeId{1, "full"});
    REQUIRE(std::string(idFull.getNamespaceUri()) == "namespace");
    REQUIRE(idFull.getServerIndex() == 1);

    REQUIRE(idLocal == idLocal);
    REQUIRE(idLocal != idFull);
}

TEST_CASE("Variant") {
    SECTION("Empty variant") {
        Variant varEmpty;
        REQUIRE(varEmpty.isEmpty());
        REQUIRE_FALSE(varEmpty.isScalar());
        REQUIRE_FALSE(varEmpty.isArray());
        REQUIRE(varEmpty.getVariantType() == std::nullopt);
        REQUIRE(varEmpty.getArrayLength() == 0);
        REQUIRE(varEmpty.getArrayDimensions().empty());

        SECTION("Type checks") {
            REQUIRE_FALSE(varEmpty.isType(Type::Boolean));
            REQUIRE_FALSE(varEmpty.isType(Type::Int16));
            REQUIRE_FALSE(varEmpty.isType(Type::UInt16));
            REQUIRE_FALSE(varEmpty.isType(Type::Int32));
            REQUIRE_FALSE(varEmpty.isType(Type::UInt32));
            REQUIRE_FALSE(varEmpty.isType(Type::Int64));
            REQUIRE_FALSE(varEmpty.isType(Type::UInt64));
            REQUIRE_FALSE(varEmpty.isType(Type::Float));
            REQUIRE_FALSE(varEmpty.isType(Type::Double));
            // ...
        }
    }

    SECTION("Create from scalar") {
        SECTION("Assign if possible") {
            double value = 11.11;
            const auto var = Variant::fromScalar(value);
            REQUIRE(var.isScalar());
            REQUIRE(var->data == &value);
        }
        SECTION("Copy if const") {
            const double value = 11.11;
            const auto var = Variant::fromScalar(value);
            REQUIRE(var.isScalar());
            REQUIRE(var->data != &value);
        }
        SECTION("Copy rvalue") {
            auto var = Variant::fromScalar(11.11);
            REQUIRE(var.isScalar());
            REQUIRE(var.getScalar<double>() == 11.11);
        }
        SECTION("Copy if not assignable (const or conversion required)") {
            std::string value{"test"};
            const auto var = Variant::fromScalar<std::string, Type::String>(value);
            REQUIRE(var.isScalar());
            REQUIRE(var->data != &value);
        }
    }

    SECTION("Create from array") {
        SECTION("Assign if possible") {
            std::vector<double> vec{1.1, 2.2, 3.3};
            const auto var = Variant::fromArray(vec.data(), vec.size());
            REQUIRE(var.isArray());
            REQUIRE(var->data == vec.data());
        }
        SECTION("Copy if const") {
            const std::vector<double> vec{1.1, 2.2, 3.3};
            const auto var = Variant::fromArray(vec);
            REQUIRE(var.isArray());
            REQUIRE(var->data != vec.data());
        }
        SECTION("Copy from iterator") {
            const std::vector<double> vec{1.1, 2.2, 3.3};
            const auto var = Variant::fromArray(vec.begin(), vec.end());
            REQUIRE(var.isArray());
            REQUIRE(var->data != vec.data());
        }
    }

    SECTION("Set/get scalar") {
        Variant var;
        int32_t value = 5;
        var.setScalar(value);

        CHECK(var.isScalar());
        CHECK(var.isType(&UA_TYPES[UA_TYPES_INT32]));
        CHECK(var.isType(Type::Int32));
        CHECK(var.isType(NodeId{0, UA_NS0ID_INT32}));
        CHECK(var.getVariantType().value() == Type::Int32);

        REQUIRE_THROWS(var.getScalar<bool>());
        REQUIRE_THROWS(var.getScalar<int16_t>());
        REQUIRE_THROWS(var.getArrayCopy<int32_t>());
        REQUIRE(var.getScalar<int32_t>() == value);
        REQUIRE(var.getScalarCopy<int32_t>() == value);
    }

    SECTION("Set/get scalar reference") {
        Variant var;
        int value = 3;
        var.setScalar(value);
        int& ref = var.getScalar<int>();
        REQUIRE(ref == value);
        REQUIRE(&ref == &value);

        value++;
        REQUIRE(ref == value);
        ref++;
        REQUIRE(ref == value);
    }

    SECTION("Set/get mixed scalar types") {
        Variant var;

        var.setScalarCopy(static_cast<int>(11));
        REQUIRE(var.getScalar<int>() == 11);
        REQUIRE(var.getScalarCopy<int>() == 11);

        var.setScalarCopy(static_cast<float>(11.11));
        REQUIRE(var.getScalar<float>() == 11.11f);
        REQUIRE(var.getScalarCopy<float>() == 11.11f);

        var.setScalarCopy(static_cast<short>(1));
        REQUIRE(var.getScalar<short>() == 1);
        REQUIRE(var.getScalarCopy<short>() == 1);
    }

    SECTION("Set/get wrapped scalar types") {
        Variant var;

        {
            TypeWrapper<int32_t, UA_TYPES_INT32> value(10);
            var.setScalar(value);
            REQUIRE(var.getScalar<int32_t>() == 10);
            REQUIRE(var.getScalarCopy<int32_t>() == 10);
        }

        {
            LocalizedText value("en-US", "text");
            var.setScalar(value);
            REQUIRE(var.getScalarCopy<LocalizedText>() == value);
        }
    }

    SECTION("Set/get array (copy)") {
        Variant var;
        std::vector<float> value{0, 1, 2, 3, 4, 5};
        var.setArrayCopy(value);

        CHECK(var.isArray());
        CHECK(var.isType(Type::Float));
        CHECK(var.isType(NodeId{0, UA_NS0ID_FLOAT}));
        CHECK(var.getVariantType().value() == Type::Float);
        CHECK(var.getArrayLength() == value.size());
        CHECK(var.handle()->data != value.data());

        REQUIRE_THROWS(var.getArrayCopy<int32_t>());
        REQUIRE_THROWS(var.getArrayCopy<bool>());
        REQUIRE(var.getArrayCopy<float>() == value);
    }

    SECTION("Set/get array reference") {
        Variant var;
        std::vector<float> value{0, 1, 2};
        var.setArray(value);
        REQUIRE(var.getArray<float>() == value.data());
        REQUIRE(var.getArrayCopy<float>() == value);

        std::vector<float> valueChanged({3, 4, 5});
        value.assign(valueChanged.begin(), valueChanged.end());
        REQUIRE(var.getArrayCopy<float>() == valueChanged);
    }

    SECTION("Set array of native strings") {
        Variant var;
        std::array array{
            detail::allocUaString("item1"),
            detail::allocUaString("item2"),
            detail::allocUaString("item3"),
        };

        var.setArray<UA_String, Type::String>(array.data(), array.size());
        REQUIRE(var.getArrayLength() == array.size());
        REQUIRE(var.handle()->data == array.data());

        UA_clear(&array[0], &UA_TYPES[UA_TYPES_STRING]);
        UA_clear(&array[1], &UA_TYPES[UA_TYPES_STRING]);
        UA_clear(&array[2], &UA_TYPES[UA_TYPES_STRING]);
    }

    SECTION("Set/get array of strings") {
        Variant var;
        std::vector<std::string> value{"a", "b", "c"};
        var.setArrayCopy<std::string, Type::String>(value);

        CHECK(var.isArray());
        CHECK(var.isType(Type::String));
        CHECK(var.isType(NodeId{0, UA_NS0ID_STRING}));
        CHECK(var.getVariantType().value() == Type::String);

        REQUIRE_THROWS(var.getScalarCopy<std::string>());
        REQUIRE_THROWS(var.getArrayCopy<int32_t>());
        REQUIRE_THROWS(var.getArrayCopy<bool>());
        REQUIRE(var.getArrayCopy<std::string>() == value);
    }
}

TEST_CASE("DataValue") {
    SECTION("Create from scalar") {
        REQUIRE(DataValue::fromScalar(5).getValue().value().getScalar<int>() == 5);
    }

    SECTION("Create from aray") {
        std::vector<int> vec{1, 2, 3};
        REQUIRE(DataValue::fromArray(vec).getValue().value().getArrayCopy<int>() == vec);
    }

    SECTION("Empty") {
        DataValue dv{};
        CHECK(dv.getValuePtr() == nullptr);
        CHECK_FALSE(dv.getValue().has_value());
        CHECK_FALSE(dv.getSourceTimestamp().has_value());
        CHECK_FALSE(dv.getServerTimestamp().has_value());
        CHECK_FALSE(dv.getSourcePicoseconds().has_value());
        CHECK_FALSE(dv.getServerPicoseconds().has_value());
        CHECK_FALSE(dv.getStatusCode().has_value());
    }

    SECTION("Constructor with all optional parameter empty") {
        DataValue dv({}, {}, {}, {}, {}, {});
        CHECK(dv.getValuePtr() != nullptr);
        CHECK(dv.getValuePtr()->handle() == &dv.handle()->value);
        CHECK(dv.getValue().has_value());
        CHECK_FALSE(dv.getSourceTimestamp().has_value());
        CHECK_FALSE(dv.getServerTimestamp().has_value());
        CHECK_FALSE(dv.getSourcePicoseconds().has_value());
        CHECK_FALSE(dv.getServerPicoseconds().has_value());
        CHECK_FALSE(dv.getStatusCode().has_value());
    }

    SECTION("Constructor with all optional parameter specified") {
        Variant var;
        DataValue dv(var, DateTime{1}, DateTime{2}, 3, 4, 5);
        CHECK(dv.getValuePtr() != nullptr);
        CHECK(dv.getValuePtr()->handle() == &dv.handle()->value);
        CHECK(dv.getValue().has_value());
        CHECK(dv.getSourceTimestamp().value() == DateTime{1});
        CHECK(dv.getServerTimestamp().value() == DateTime{2});
        CHECK(dv.getSourcePicoseconds().value() == 3);
        CHECK(dv.getServerPicoseconds().value() == 4);
        CHECK(dv.getStatusCode().value() == 5);
    }

    SECTION("Setter methods") {
        DataValue dv;
        SECTION("Value (move)") {
            float value = 11.11f;
            Variant var;
            var.setScalar(value);
            REQUIRE(var->data == &value);
            dv.setValue(std::move(var));
            REQUIRE(dv.getValue().value().getScalar<float>() == value);
            REQUIRE(dv->value.data == &value);
        }
        SECTION("Value (copy)") {
            float value = 11.11f;
            Variant var;
            var.setScalar(value);
            dv.setValue(var);
            REQUIRE(dv.getValue().value().getScalar<float>() == value);
        }
        SECTION("Source timestamp") {
            DateTime dt{123};
            dv.setSourceTimestamp(dt);
            REQUIRE(dv.getSourceTimestamp().value() == dt);
        }
        SECTION("Server timestamp") {
            DateTime dt{456};
            dv.setServerTimestamp(dt);
            REQUIRE(dv.getServerTimestamp().value() == dt);
        }
        SECTION("Source picoseconds") {
            const uint16_t ps = 123;
            dv.setSourcePicoseconds(ps);
            REQUIRE(dv.getSourcePicoseconds().value() == ps);
        }
        SECTION("Server picoseconds") {
            const uint16_t ps = 456;
            dv.setServerPicoseconds(ps);
            REQUIRE(dv.getServerPicoseconds().value() == ps);
        }
        SECTION("Status code") {
            const UA_StatusCode statusCode = UA_STATUSCODE_BADALREADYEXISTS;
            dv.setStatusCode(statusCode);
            REQUIRE(dv.getStatusCode().value() == statusCode);
        }
    }
}
