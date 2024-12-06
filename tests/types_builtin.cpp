#include <sstream>

#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/detail/string_utils.hpp"  // toNativeString
#include "open62541pp/types.hpp"
#include "open62541pp/ua/nodeids.hpp"

using namespace opcua;

TEST_CASE("StatusCode") {
    SUBCASE("Good") {
        StatusCode code;
        CHECK(code == UA_STATUSCODE_GOOD);
        CHECK(code.get() == UA_STATUSCODE_GOOD);
        CHECK(code.name() == "Good");
        CHECK(code.isGood());
        CHECK(!code.isUncertain());
        CHECK(!code.isBad());
        CHECK_NOTHROW(code.throwIfBad());
    }

#ifdef UA_STATUSCODE_UNCERTAIN
    SUBCASE("Uncertain") {
        StatusCode code(UA_STATUSCODE_UNCERTAIN);
        CHECK(code == UA_STATUSCODE_UNCERTAIN);
        CHECK(code.get() == UA_STATUSCODE_UNCERTAIN);
        CHECK(code.name() == "Uncertain");
        CHECK(!code.isGood());
        CHECK(code.isUncertain());
        CHECK(!code.isBad());
        CHECK_NOTHROW(code.throwIfBad());
    }
#endif

    SUBCASE("Bad") {
        StatusCode code(UA_STATUSCODE_BADTIMEOUT);
        CHECK(code == UA_STATUSCODE_BADTIMEOUT);
        CHECK(code.get() == UA_STATUSCODE_BADTIMEOUT);
        CHECK(code.name() == "BadTimeout");
        CHECK(!code.isGood());
        CHECK(!code.isUncertain());
        CHECK(code.isBad());
        CHECK_THROWS_AS_MESSAGE(code.throwIfBad(), BadStatus, "BadTimeout");
    }
}

TEST_CASE_TEMPLATE("StringLikeMixin constructors", T, String, const String) {
    SUBCASE("Default") {
        T str;
        CHECK(str.size() == 0);
        CHECK(str.length() == 0);
        CHECK(str.empty());
        CHECK(str.data() == nullptr);
    }

    SUBCASE("From empty iterator pair") {
        std::string_view sv;
        T str(sv.begin(), sv.end());
        CHECK(str.size() == 0);
        CHECK(str.length() == 0);
        CHECK(str.empty());
        CHECK(str.data() == nullptr);
    }

    SUBCASE("From iterator pair") {
        std::string_view sv("abc");
        T str(sv.begin(), sv.end());
        CHECK(str.size() == 3);
        CHECK(str.length() == 3);
        CHECK_FALSE(str.empty());
        CHECK(str.data() != nullptr);
        CHECK(std::string_view(str.data(), str.size()) == sv);
    }

    SUBCASE("From iterator pair (input iterator, single-pass)") {
        std::istringstream ss("abc");  // allows only single-pass reading
        std::istream_iterator<char> first(ss), last;
        T str(first, last);
        CHECK(str.size() == 3);
        CHECK(str.length() == 3);
        CHECK_FALSE(str.empty());
        CHECK(str.data() != nullptr);
        CHECK(std::string_view(str.data(), str.size()) == "abc");
    }

    SUBCASE("From initializer list") {
        T str{'a', 'b', 'c'};
        CHECK(str.size() == 3);
        CHECK(str.length() == 3);
        CHECK_FALSE(str.empty());
        CHECK(str.data() != nullptr);
        CHECK(std::string_view(str.data(), str.size()) == "abc");
    }
}

TEST_CASE_TEMPLATE("StringLikeMixin element access", T, String, const String) {
    T str{'a', 'b', 'c'};

    SUBCASE("operator[]") {
        CHECK(str[0] == 'a');
        CHECK(str[1] == 'b');
        CHECK(str[2] == 'c');
    }

    SUBCASE("front() and back()") {
        CHECK(str.front() == 'a');
        CHECK(str.back() == 'c');
    }
}

TEST_CASE_TEMPLATE("StringLikeMixin iterators", T, String, const String) {
    T str{'a', 'b', 'c'};

    SUBCASE("begin(), end() iterators") {
        CHECK(*str.begin() == 'a');
        CHECK(*(str.begin() + 1) == 'b');
        CHECK(*(str.begin() + 2) == 'c');
        CHECK(str.end() - str.begin() == 3);

        std::string result;
        for (auto it = str.begin(); it != str.end(); ++it) {
            result += *it;
        }
        CHECK(result == "abc");
    }

    SUBCASE("rbegin(), rend() reverse iterators") {
        CHECK(*str.rbegin() == 'c');
        CHECK(*(str.rbegin() + 1) == 'b');
        CHECK(*(str.rbegin() + 2) == 'a');
        CHECK(str.rend() - str.rbegin() == 3);

        std::string result;
        for (auto it = str.rbegin(); it != str.rend(); ++it) {
            result += *it;
        }
        CHECK(result == "cba");
    }
}

TEST_CASE_TEMPLATE("StringLike constructors", T, String, XmlElement) {
    SUBCASE("From const char*") {
        T str("hello");
        CHECK(str.size() == 5);
        CHECK(str.length() == 5);
        CHECK_FALSE(str.empty());
        CHECK(str.data() != nullptr);
        CHECK(std::string_view(str.data(), str.size()) == "hello");
    }

    SUBCASE("From std::string_view") {
        std::string_view sv = "world";
        T str(sv);
        CHECK(str.size() == 5);
        CHECK(str.length() == 5);
        CHECK_FALSE(str.empty());
        CHECK(str.data() != nullptr);
        CHECK(std::string_view(str.data(), str.size()) == "world");
    }

    SUBCASE("From empty string") {
        T str("");
        CHECK(str.size() == 0);
        CHECK(str.length() == 0);
        CHECK(str.empty());
        CHECK(str.data() != nullptr);
    }
}

TEST_CASE_TEMPLATE("StringLike implicit conversion to string_view", T, String, XmlElement) {
    T str("test123");
    std::string_view view = str;
    CHECK(view == "test123");
}

TEST_CASE_TEMPLATE("StringLike explicit conversion to string_view", T, ByteString) {
    T str("test123");
    CHECK(static_cast<std::string_view>(str) == "test123");
}

TEST_CASE_TEMPLATE("StringLike equality overloads", T, String, ByteString, XmlElement) {
    CHECK(T("test") == T("test"));
    CHECK(T("test") != T());
}

TEST_CASE_TEMPLATE("StringLike equality overloads with std::string_view", T, String, ByteString) {
    CHECK(T("test") == std::string("test"));
    CHECK(T("test") != std::string("abc"));
    CHECK(std::string("test") == T("test"));
    CHECK(std::string("test") != T("abc"));
}

TEST_CASE_TEMPLATE("StringLike ostream overloads", T, String, XmlElement) {
    std::ostringstream ss;
    ss << T("test123");
    CHECK(ss.str() == "test123");
}

TEST_CASE("ByteString") {
    SUBCASE("Construct from string") {
        const ByteString bs("XYZ");
        CHECK(bs->length == 3);
        CHECK(bs->data[0] == 88);
        CHECK(bs->data[1] == 89);
        CHECK(bs->data[2] == 90);
    }

    SUBCASE("Construct from vector") {
        const ByteString bs({88, 89, 90});
        CHECK(bs->length == 3);
        CHECK(bs->data[0] == 88);
        CHECK(bs->data[1] == 89);
        CHECK(bs->data[2] == 90);
    }

#if UAPP_OPEN62541_VER_GE(1, 1)
    SUBCASE("fromBase64 / to Base64") {
        CHECK(ByteString::fromBase64("dGVzdDEyMw==") == ByteString("test123"));
        CHECK(ByteString("test123").toBase64() == "dGVzdDEyMw==");
    }
#endif
}

TEST_CASE("Guid") {
    SUBCASE("Construct") {
        const Guid wrapper(11, 22, 33, {0, 1, 2, 3, 4, 5, 6, 7});
        CHECK(wrapper.handle()->data1 == 11);
        CHECK(wrapper.handle()->data2 == 22);
        CHECK(wrapper.handle()->data3 == 33);
        CHECK(wrapper.handle()->data4[0] == 0);
        CHECK(wrapper.handle()->data4[1] == 1);
        CHECK(wrapper.handle()->data4[2] == 2);
        CHECK(wrapper.handle()->data4[3] == 3);
        CHECK(wrapper.handle()->data4[4] == 4);
        CHECK(wrapper.handle()->data4[5] == 5);
        CHECK(wrapper.handle()->data4[6] == 6);
        CHECK(wrapper.handle()->data4[7] == 7);
    }

    SUBCASE("Construct from single array") {
        std::array<UA_Byte, 16> data{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        const Guid wrapper(data);
        CHECK(wrapper.handle()->data1 == 0x00010203);
        CHECK(wrapper.handle()->data2 == 0x0405);
        CHECK(wrapper.handle()->data3 == 0x0607);
        CHECK(wrapper.handle()->data4[0] == 8);
        CHECK(wrapper.handle()->data4[1] == 9);
        CHECK(wrapper.handle()->data4[2] == 10);
        CHECK(wrapper.handle()->data4[3] == 11);
        CHECK(wrapper.handle()->data4[4] == 12);
        CHECK(wrapper.handle()->data4[5] == 13);
        CHECK(wrapper.handle()->data4[6] == 14);
        CHECK(wrapper.handle()->data4[7] == 15);
    }

    SUBCASE("Random") {
        CHECK(Guid::random() != Guid());
        CHECK(Guid::random() != Guid::random());
    }

    SUBCASE("toString") {
        {
            const Guid guid{};
            CHECK(guid.toString() == "00000000-0000-0000-0000-000000000000");
        }
        {
            const Guid guid{
                0x12345678, 0x1234, 0x5678, {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF}
            };
            CHECK(guid.toString() == "12345678-1234-5678-1234-567890ABCDEF");
        }
    }

    SUBCASE("ostream overload") {
        std::ostringstream ss;
        ss << Guid{};
        CHECK(ss.str() == "00000000-0000-0000-0000-000000000000");
    }
}

TEST_CASE("DateTime") {
    SUBCASE("Empty") {
        const DateTime dt;
        CHECK(dt.get() == 0);
        CHECK(*dt.handle() == 0);
        // UA time starts before Unix time -> 0
        CHECK(dt.toTimePoint().time_since_epoch().count() == 0);
        CHECK(dt.toUnixTime() == 0);

        const auto dts = dt.toStruct();
        CHECK(dts.nanoSec == 0);
        CHECK(dts.microSec == 0);
        CHECK(dts.milliSec == 0);
        CHECK(dts.sec == 0);
        CHECK(dts.min == 0);
        CHECK(dts.hour == 0);
        CHECK(dts.day == 1);
        CHECK(dts.month == 1);
        CHECK(dts.year == 1601);
    }

    SUBCASE("Static methods") {
        CHECK_NOTHROW(DateTime::localTimeUtcOffset());
    }

    SUBCASE("From std::chrono::time_point") {
        using namespace std::chrono;

        const auto now = system_clock::now();
        const int64_t secSinceEpoch = duration_cast<seconds>(now.time_since_epoch()).count();
        const int64_t nsecSinceEpoch = duration_cast<nanoseconds>(now.time_since_epoch()).count();

        const DateTime dt(now);
        CHECK(dt.get() == (nsecSinceEpoch / 100) + UA_DATETIME_UNIX_EPOCH);
        CHECK(dt.toUnixTime() == secSinceEpoch);
    }

    SUBCASE("Format") {
        CHECK(DateTime().format("%Y-%m-%d %H:%M:%S") == "1970-01-01 00:00:00");
    }

    SUBCASE("Comparison") {
        const auto zero = DateTime(0);
        const auto now = DateTime::now();
        CHECK(zero != now);
        CHECK(zero < now);
    }
}

TEST_CASE("NodeId") {
    SUBCASE("Numeric identifier") {
        NodeId id(1, 123);
        CHECK(id.identifierType() == NodeIdType::Numeric);
        CHECK(id.namespaceIndex() == 1);
        CHECK(id.identifier<uint32_t>() == 123);
        CHECK(id.identifierIf<uint32_t>() != nullptr);
        CHECK(*id.identifierIf<uint32_t>() == 123);
    }

    SUBCASE("String identifier") {
        String str("Test456");
        NodeId id(2, str);
        CHECK(id.identifierType() == NodeIdType::String);
        CHECK(id.namespaceIndex() == 2);
        CHECK(id.identifier<String>() == str);
        CHECK(id.identifierIf<String>() != nullptr);
        CHECK(*id.identifierIf<String>() == str);
    }

    SUBCASE("Guid identifier") {
        Guid guid = Guid::random();
        NodeId id(3, guid);
        CHECK(id.identifierType() == NodeIdType::Guid);
        CHECK(id.namespaceIndex() == 3);
        CHECK(id.identifier<Guid>() == guid);
        CHECK(id.identifierIf<Guid>() != nullptr);
        CHECK(*id.identifierIf<Guid>() == guid);
    }

    SUBCASE("ByteString identifier") {
        ByteString byteStr("Test789");
        NodeId id(4, byteStr);
        CHECK(id.identifierType() == NodeIdType::ByteString);
        CHECK(id.namespaceIndex() == 4);
        CHECK(id.identifier<ByteString>() == byteStr);
        CHECK(id.identifierIf<ByteString>() != nullptr);
        CHECK(*id.identifierIf<ByteString>() == byteStr);
    }

    SUBCASE("Construct from node id enums") {
        CHECK(NodeId(DataTypeId::Boolean) == NodeId(0, UA_NS0ID_BOOLEAN));
        CHECK(NodeId(ReferenceTypeId::References) == NodeId(0, UA_NS0ID_REFERENCES));
        CHECK(NodeId(ObjectTypeId::BaseObjectType) == NodeId(0, UA_NS0ID_BASEOBJECTTYPE));
        CHECK(NodeId(VariableTypeId::BaseVariableType) == NodeId(0, UA_NS0ID_BASEVARIABLETYPE));
        CHECK(NodeId(ObjectId::RootFolder) == NodeId(0, UA_NS0ID_ROOTFOLDER));
        CHECK(NodeId(VariableId::LocalTime) == NodeId(0, UA_NS0ID_LOCALTIME));
        CHECK(NodeId(MethodId::AddCommentMethodType) == NodeId(0, UA_NS0ID_ADDCOMMENTMETHODTYPE));
    }

    SUBCASE("Get invalid identifier type") {
        NodeId id(1, 123);
        CHECK(id.identifierIf<uint32_t>() != nullptr);
        CHECK(id.identifierIf<String>() == nullptr);
        CHECK(id.identifierIf<Guid>() == nullptr);
        CHECK(id.identifierIf<ByteString>() == nullptr);
        CHECK_NOTHROW(id.identifier<uint32_t>());
        CHECK_THROWS_AS(id.identifier<String>(), TypeError);
        CHECK_THROWS_AS(id.identifier<Guid>(), TypeError);
        CHECK_THROWS_AS(id.identifier<ByteString>(), TypeError);
    }

    SUBCASE("Comparison") {
        CHECK(NodeId(0, 1) == NodeId(0, 1));
        CHECK(NodeId(0, 1) <= NodeId(0, 1));
        CHECK(NodeId(0, 1) >= NodeId(0, 1));
        CHECK(NodeId(0, 1) != NodeId(0, 2));
        CHECK(NodeId(0, 1) != NodeId(1, 1));
        CHECK(NodeId(0, "a") == NodeId(0, "a"));
        CHECK(NodeId(0, "a") != NodeId(0, "b"));
        CHECK(NodeId(0, "a") != NodeId(1, "a"));

        // namespace index is compared before identifier
        CHECK(NodeId(0, 1) < NodeId(1, 0));
        CHECK(NodeId(0, 1) < NodeId(0, 2));

        CHECK(NodeId(1, "a") < NodeId(1, "b"));
        CHECK(NodeId(1, "b") > NodeId(1, "a"));
    }

    SUBCASE("isNull") {
        CHECK(NodeId().isNull());
        CHECK_FALSE(NodeId(0, 1).isNull());
    }

    SUBCASE("Hash") {
        CHECK(NodeId(0, 1).hash() == NodeId(0, 1).hash());
        CHECK(NodeId(0, 1).hash() != NodeId(0, 2).hash());
        CHECK(NodeId(0, 1).hash() != NodeId(1, 1).hash());
    }

    SUBCASE("toString") {
        CHECK(NodeId(0, 13).toString() == "i=13");
        CHECK(NodeId(10, 1).toString() == "ns=10;i=1");
        CHECK(NodeId(10, "Hello:World").toString() == "ns=10;s=Hello:World");
        CHECK(NodeId(0, Guid()).toString() == "g=00000000-0000-0000-0000-000000000000");
#if UAPP_OPEN62541_VER_GE(1, 1)
        CHECK(NodeId(1, ByteString("test123")).toString() == "ns=1;b=dGVzdDEyMw==");
#endif
    }

    SUBCASE("std::hash specialization") {
        const NodeId id(1, "Test123");
        CHECK(std::hash<NodeId>{}(id) == id.hash());
    }
}

TEST_CASE("ExpandedNodeId") {
    ExpandedNodeId idLocal({1, "local"}, {}, 0);
    CHECK(idLocal.isLocal());
    CHECK(idLocal.nodeId() == NodeId{1, "local"});
    CHECK(idLocal.nodeId().handle() == &idLocal.handle()->nodeId);  // return ref
    CHECK(idLocal.namespaceUri().empty());
    CHECK(idLocal.serverIndex() == 0);

    ExpandedNodeId idFull({1, "full"}, "namespace", 1);
    CHECK(idFull.nodeId() == NodeId{1, "full"});
    CHECK(std::string(idFull.namespaceUri()) == "namespace");
    CHECK(idFull.serverIndex() == 1);

    CHECK(idLocal == idLocal);
    CHECK(idLocal != idFull);

    SUBCASE("Hash") {
        CHECK(ExpandedNodeId().hash() == ExpandedNodeId().hash());
        CHECK(ExpandedNodeId().hash() != idLocal.hash());
        CHECK(ExpandedNodeId().hash() != idFull.hash());
    }

    SUBCASE("toString") {
        CHECK_EQ(ExpandedNodeId({2, 10157}).toString(), "ns=2;i=10157");
        CHECK_EQ(
            ExpandedNodeId({2, 10157}, "http://test.org/UA/Data/", 0).toString(),
            "nsu=http://test.org/UA/Data/;ns=2;i=10157"
        );
        CHECK_EQ(
            ExpandedNodeId({2, 10157}, "http://test.org/UA/Data/", 1).toString(),
            "svr=1;nsu=http://test.org/UA/Data/;ns=2;i=10157"
        );
    }

    SUBCASE("std::hash specialization") {
        CHECK(std::hash<ExpandedNodeId>()(idLocal) == idLocal.hash());
    }
}

TEST_CASE("Variant") {
    SUBCASE("Empty variant") {
        Variant var;
        CHECK(var.isEmpty());
        CHECK(!var.isScalar());
        CHECK(!var.isArray());
        CHECK(var.type() == nullptr);
        CHECK(var.data() == nullptr);
        CHECK(std::as_const(var).data() == nullptr);
        CHECK(var.arrayLength() == 0);
        CHECK(var.arrayDimensions().empty());
        CHECK_THROWS(var.getScalar<int>());
        CHECK_THROWS(var.getScalarCopy<int>());
        CHECK_THROWS(var.getArray<int>());
        CHECK_THROWS(var.getArrayCopy<int>());
    }

    SUBCASE("Type checks") {
        Variant var;
        CHECK_FALSE(var.isType(UA_TYPES[UA_TYPES_STRING]));
        CHECK_FALSE(var.isType(DataTypeId::String));
        CHECK_FALSE(var.isType<String>());
        CHECK(var.type() == nullptr);

        var->type = &UA_TYPES[UA_TYPES_STRING];
        CHECK(var.isType(UA_TYPES[UA_TYPES_STRING]));
        CHECK(var.isType(DataTypeId::String));
        CHECK(var.isType<String>());
        CHECK(var.type() == &UA_TYPES[UA_TYPES_STRING]);
    }

    SUBCASE("Set/get scalar") {
        Variant var;
        int32_t value = 5;
        var.setScalar(value);
        CHECK(var.isScalar());
        CHECK(var.data() == &value);
        CHECK(&var.getScalar<int32_t>() == &value);
        CHECK(&std::as_const(var).getScalar<int32_t>() == &value);
        CHECK(var.getScalarCopy<int32_t>() == value);
    }

    SUBCASE("Set/get wrapped scalar types") {
        Variant var;
        LocalizedText value("en-US", "text");
        var.setScalar(value);
        CHECK(var.getScalar<LocalizedText>() == value);
        CHECK(var.getScalarCopy<LocalizedText>() == value);
    }

    SUBCASE("Set/get scalar (copy)") {
        Variant var;
        var.setScalarCopy(11.11);
        CHECK(var.getScalar<double>() == 11.11);
        CHECK(var.getScalarCopy<double>() == 11.11);
    }

    SUBCASE("Set/get array") {
        Variant var;
        std::vector<float> array{0, 1, 2};
        var.setArray(array);
        CHECK(var.data() == array.data());
        CHECK(var.getArray<float>().data() == array.data());
        CHECK(std::as_const(var).getArray<float>().data() == array.data());
        CHECK(var.getArrayCopy<float>() == array);
    }

    SUBCASE("Set array of native strings") {
        Variant var;
        std::array array{
            detail::toNativeString("item1"),
            detail::toNativeString("item2"),
            detail::toNativeString("item3"),
        };
        var.setArray(Span{array.data(), array.size()}, UA_TYPES[UA_TYPES_STRING]);
        CHECK(var.data() == array.data());
        CHECK(var.arrayLength() == array.size());
    }

    SUBCASE("Set array of string wrapper") {
        Variant var;
        std::vector<String> array{String{"item1"}, String{"item2"}, String{"item3"}};
        var.setArray(array);
        CHECK(var.data() == array.data());
        CHECK(var.arrayLength() == array.size());
        CHECK(var.getArray<String>().data() == array.data());
    }

    SUBCASE("Set/get array of std::string (conversion)") {
        Variant var;
        std::vector<std::string> value{"a", "b", "c"};
        var.setArrayCopy(value);

        CHECK(var.isArray());
        CHECK(var.isType(NodeId{0, UA_NS0ID_STRING}));
        CHECK(var.type() == &UA_TYPES[UA_TYPES_STRING]);

        CHECK_THROWS(var.getScalarCopy<std::string>());
        CHECK_THROWS(var.getArrayCopy<int32_t>());
        CHECK_THROWS(var.getArrayCopy<bool>());
        CHECK(var.getArrayCopy<std::string>() == value);
    }

    SUBCASE("Set/get array (copy)") {
        Variant var;
        std::vector<float> array{0, 1, 2, 3, 4, 5};
        var.setArrayCopy(array);

        CHECK(var.isArray());
        CHECK(var.isType(NodeId{0, UA_NS0ID_FLOAT}));
        CHECK(var.type() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(var.data() != array.data());
        CHECK(var.arrayLength() == array.size());

        CHECK_THROWS(var.getArrayCopy<int32_t>());
        CHECK_THROWS(var.getArrayCopy<bool>());
        CHECK(var.getArrayCopy<float>() == array);
    }

    SUBCASE("Set array from initializer list (copy)") {
        Variant var;
        var.setArrayCopy(Span<const int>{1, 2, 3});  // TODO: avoid manual template types
    }

    SUBCASE("Set/get array with std::vector<bool> (copy)") {
        // std::vector<bool> is a possibly space optimized template specialization which caused
        // several problems: https://github.com/open62541pp/open62541pp/issues/164
        Variant var;
        std::vector<bool> array{true, false, true};

        SUBCASE("From vector") {
            var = Variant::fromArray(array);
        }

        SUBCASE("Copy from iterator") {
            var.setArrayCopy(array.begin(), array.end());
        }

        SUBCASE("Copy directly") {
            var.setArrayCopy(array);
        }

        CHECK(var.arrayLength() == array.size());
        CHECK(var.isType<bool>());
        CHECK(var.getArrayCopy<bool>() == array);
    }

    SUBCASE("Set/get non-builtin data types") {
        using CustomType = UA_WriteValue;
        const auto& dt = UA_TYPES[UA_TYPES_WRITEVALUE];

        Variant var;
        CustomType value{};
        value.attributeId = 1;

        SUBCASE("Scalar") {
            var.setScalar(value, dt);
            CHECK(var.isScalar());
            CHECK(var.type() == &dt);
            CHECK(var.data() == &value);
            CHECK(var.getScalar<CustomType>().attributeId == 1);
        }

        SUBCASE("Scalar (copy)") {
            var.setScalarCopy(value, dt);
            CHECK(var.isScalar());
            CHECK(var.type() == &dt);
            CHECK(var.data() != &value);
            CHECK(var.getScalar<CustomType>().attributeId == 1);
        }

        std::vector<CustomType> array(3);

        SUBCASE("Array") {
            var.setArray(array, dt);
            CHECK(var.isArray());
            CHECK(var.type() == &dt);
            CHECK(var.data() == array.data());
            CHECK(var.arrayLength() == 3);
            CHECK(var.getArray<CustomType>().data() == array.data());
        }

        SUBCASE("Array (copy)") {
            var.setArrayCopy(array, dt);
            CHECK(var.isArray());
            CHECK(var.type() == &dt);
            CHECK(var.data() != array.data());
            CHECK(var.arrayLength() == 3);
            CHECK(var.getArray<CustomType>().data() != array.data());
        }
    }

    SUBCASE("getScalar (lvalue & rvalue)") {
        auto var = Variant::fromScalar("test");
        void* data = var.getScalar<String>()->data;

        String str;
        SUBCASE("rvalue") {
            str = var.getScalar<String>();
            CHECK(str->data != data);  // copy
        }
        SUBCASE("const rvalue") {
            str = std::as_const(var).getScalar<String>();
            CHECK(str->data != data);  // copy
        }
        SUBCASE("lvalue") {
            str = std::move(var).getScalar<String>();
            CHECK(str->data == data);  // move
        }
        SUBCASE("const lvalue") {
            str = std::move(std::as_const(var)).getScalar<String>();
            CHECK(str->data != data);  // can not move const -> copy
        }
    }

    SUBCASE("Set/get value") {
        SUBCASE("Set/get scalar") {
            Variant var;
            int32_t value = 5;
            var.setValue(value);
            CHECK(var.isScalar());
            CHECK(var.data() == &value);
            CHECK(&var.getScalar<int32_t>() == &value);
            CHECK(&std::as_const(var).getScalar<int32_t>() == &value);
            CHECK(var.getScalarCopy<int32_t>() == value);
        }

        SUBCASE("Set/get wrapped scalar types") {
            Variant var;
            LocalizedText value("en-US", "text");
            var.setValue(value);
            CHECK(var.getScalar<LocalizedText>() == value);
            CHECK(var.getScalarCopy<LocalizedText>() == value);
        }

        SUBCASE("Set/get scalar (copy)") {
            Variant var;
            var.setValueCopy(11.11);
            CHECK(var.getScalar<double>() == 11.11);
            CHECK(var.getScalarCopy<double>() == 11.11);
        }

        SUBCASE("Set/get array") {
            Variant var;
            std::vector<float> array{0, 1, 2};
            var.setValue(array);
            CHECK(var.data() == array.data());
            CHECK(var.getArray<float>().data() == array.data());
            CHECK(std::as_const(var).getArray<float>().data() == array.data());
            CHECK(var.getArrayCopy<float>() == array);
        }

        SUBCASE("Set array of native strings") {
            Variant var;
            std::array array{
                detail::toNativeString("item1"),
                detail::toNativeString("item2"),
                detail::toNativeString("item3"),
            };
            var.setValue(Span{array.data(), array.size()}, UA_TYPES[UA_TYPES_STRING]);
            CHECK(var.data() == array.data());
            CHECK(var.arrayLength() == array.size());
        }

        SUBCASE("Set array of string wrapper") {
            Variant var;
            std::vector<String> array{String{"item1"}, String{"item2"}, String{"item3"}};
            var.setValue(array);
            CHECK(var.data() == array.data());
            CHECK(var.arrayLength() == array.size());
            CHECK(var.getArray<String>().data() == array.data());
        }

        SUBCASE("Set/get array of std::string (conversion)") {
            Variant var;
            std::vector<std::string> value{"a", "b", "c"};
            var.setValueCopy(value);

            CHECK(var.isArray());
            CHECK(var.isType(NodeId{0, UA_NS0ID_STRING}));
            CHECK(var.type() == &UA_TYPES[UA_TYPES_STRING]);

            CHECK_THROWS(var.getScalarCopy<std::string>());
            CHECK_THROWS(var.getArrayCopy<int32_t>());
            CHECK_THROWS(var.getArrayCopy<bool>());
            CHECK(var.getArrayCopy<std::string>() == value);
        }

        SUBCASE("Set/get array (copy)") {
            Variant var;
            std::vector<float> array{0, 1, 2, 3, 4, 5};
            var.setValueCopy(array);

            CHECK(var.isArray());
            CHECK(var.isType(NodeId{0, UA_NS0ID_FLOAT}));
            CHECK(var.type() == &UA_TYPES[UA_TYPES_FLOAT]);
            CHECK(var.data() != array.data());
            CHECK(var.arrayLength() == array.size());

            CHECK_THROWS(var.getArrayCopy<int32_t>());
            CHECK_THROWS(var.getArrayCopy<bool>());
            CHECK(var.getArrayCopy<float>() == array);
        }

        SUBCASE("Set array from initializer list (copy)") {
            Variant var;
            var.setValueCopy(Span<const int>{1, 2, 3});  // TODO: avoid manual template types
        }

        SUBCASE("Set/get array with std::vector<bool> (copy)") {
            // std::vector<bool> is a possibly space optimized template specialization which caused
            // several problems: https://github.com/open62541pp/open62541pp/issues/164
            Variant var;
            std::vector<bool> array{true, false, true};

            SUBCASE("From vector") {
                var = Variant{array};
            }

            SUBCASE("Copy from iterator") {
                var.setValueCopy(array.begin(), array.end());
            }

            SUBCASE("Copy directly") {
                var.setValueCopy(array);
            }

            CHECK(var.arrayLength() == array.size());
            CHECK(var.isType<bool>());
            CHECK(var.getArrayCopy<bool>() == array);
        }

        SUBCASE("Set/get non-builtin data types") {
            using CustomType = UA_WriteValue;
            const auto& dt = UA_TYPES[UA_TYPES_WRITEVALUE];

            Variant var;
            CustomType value{};
            value.attributeId = 1;

            SUBCASE("Scalar") {
                var.setValue(value, dt);
                CHECK(var.isScalar());
                CHECK(var.type() == &dt);
                CHECK(var.data() == &value);
                CHECK(var.getScalar<CustomType>().attributeId == 1);
            }

            SUBCASE("Scalar (copy)") {
                var.setValueCopy(value, dt);
                CHECK(var.isScalar());
                CHECK(var.type() == &dt);
                CHECK(var.data() != &value);
                CHECK(var.getScalar<CustomType>().attributeId == 1);
            }

            std::vector<CustomType> array(3);

            SUBCASE("Array") {
                var.setValue(array, dt);
                CHECK(var.isArray());
                CHECK(var.type() == &dt);
                CHECK(var.data() == array.data());
                CHECK(var.arrayLength() == 3);
                CHECK(var.getArray<CustomType>().data() == array.data());
            }

            SUBCASE("Array (copy)") {
                var.setValueCopy(array, dt);
                CHECK(var.isArray());
                CHECK(var.type() == &dt);
                CHECK(var.data() != array.data());
                CHECK(var.arrayLength() == 3);
                CHECK(var.getArray<CustomType>().data() != array.data());
            }
        }

        SUBCASE("getScalar (lvalue & rvalue)") {
            auto var = Variant{"test"};
            void* data = var.getScalar<String>()->data;

            String str;
            SUBCASE("rvalue") {
                str = var.getScalar<String>();
                CHECK(str->data != data);  // copy
            }
            SUBCASE("const rvalue") {
                str = std::as_const(var).getScalar<String>();
                CHECK(str->data != data);  // copy
            }
            SUBCASE("lvalue") {
                str = std::move(var).getScalar<String>();
                CHECK(str->data == data);  // move
            }
            SUBCASE("const lvalue") {
                str = std::move(std::as_const(var)).getScalar<String>();
                CHECK(str->data != data);  // can not move const -> copy
            }
        }
    }
}

TEST_CASE("DataValue") {
    SUBCASE("Create from scalar") {
        CHECK(DataValue::fromScalar(5).value().getScalar<int>() == 5);
    }

    SUBCASE("Create from array") {
        std::vector<int> vec{1, 2, 3};
        CHECK(DataValue::fromArray(vec).value().getArrayCopy<int>() == vec);
    }

    SUBCASE("Empty") {
        DataValue dv{};
        CHECK_FALSE(dv.hasValue());
        CHECK_FALSE(dv.hasSourceTimestamp());
        CHECK_FALSE(dv.hasServerTimestamp());
        CHECK_FALSE(dv.hasSourcePicoseconds());
        CHECK_FALSE(dv.hasServerPicoseconds());
        CHECK_FALSE(dv.hasStatus());
    }

    SUBCASE("Constructor with all optional parameter empty") {
        DataValue dv({}, {}, {}, {}, {}, {});
        CHECK(dv.hasValue());
        CHECK_FALSE(dv.hasSourceTimestamp());
        CHECK_FALSE(dv.hasServerTimestamp());
        CHECK_FALSE(dv.hasSourcePicoseconds());
        CHECK_FALSE(dv.hasServerPicoseconds());
        CHECK_FALSE(dv.hasStatus());
    }

    SUBCASE("Constructor with all optional parameter specified") {
        DataValue dv(
            Variant::fromScalar(5),
            DateTime{1},
            DateTime{2},
            uint16_t{3},
            uint16_t{4},
            UA_STATUSCODE_BADINTERNALERROR
        );
        CHECK(dv.value().isScalar());
        CHECK(dv.sourceTimestamp() == DateTime{1});
        CHECK(dv.serverTimestamp() == DateTime{2});
        CHECK(dv.sourcePicoseconds() == 3);
        CHECK(dv.serverPicoseconds() == 4);
        CHECK(dv.status() == UA_STATUSCODE_BADINTERNALERROR);
    }

    SUBCASE("Setter methods") {
        DataValue dv;
        SUBCASE("Value (move)") {
            float value = 11.11f;
            Variant var;
            var.setScalar(value);
            CHECK(var->data == &value);
            dv.setValue(std::move(var));
            CHECK(dv.hasValue());
            CHECK(dv.value().getScalar<float>() == value);
            CHECK(dv->value.data == &value);
        }
        SUBCASE("Value (copy)") {
            float value = 11.11f;
            Variant var;
            var.setScalar(value);
            dv.setValue(var);
            CHECK(dv.hasValue());
            CHECK(dv.value().getScalar<float>() == value);
        }
        SUBCASE("Source timestamp") {
            DateTime dt{123};
            dv.setSourceTimestamp(dt);
            CHECK(dv.hasSourceTimestamp());
            CHECK(dv.sourceTimestamp() == dt);
        }
        SUBCASE("Server timestamp") {
            DateTime dt{456};
            dv.setServerTimestamp(dt);
            CHECK(dv.hasServerTimestamp());
            CHECK(dv.serverTimestamp() == dt);
        }
        SUBCASE("Source picoseconds") {
            const uint16_t ps = 123;
            dv.setSourcePicoseconds(ps);
            CHECK(dv.hasSourcePicoseconds());
            CHECK(dv.sourcePicoseconds() == ps);
        }
        SUBCASE("Server picoseconds") {
            const uint16_t ps = 456;
            dv.setServerPicoseconds(ps);
            CHECK(dv.hasServerPicoseconds());
            CHECK(dv.serverPicoseconds() == ps);
        }
        SUBCASE("Status") {
            const UA_StatusCode statusCode = UA_STATUSCODE_BADALREADYEXISTS;
            dv.setStatus(statusCode);
            CHECK(dv.hasStatus());
            CHECK(dv.status() == statusCode);
        }
    }

    SUBCASE("getValue (lvalue & rvalue)") {
        DataValue dv(Variant::fromScalar(11));
        void* data = dv.value().data();

        Variant var;
        SUBCASE("rvalue") {
            var = dv.value();
            CHECK(var.data() != data);  // copy
        }
        SUBCASE("const rvalue") {
            var = std::as_const(dv).value();
            CHECK(var.data() != data);  // copy
        }
        SUBCASE("lvalue") {
            var = std::move(dv).value();
            CHECK(var.data() == data);  // move
        }
        SUBCASE("const lvalue") {
            var = std::move(std::as_const(dv)).value();
            CHECK(var.data() != data);  // can not move const -> copy
        }
        CHECK(var.getScalar<int>() == 11);
    }
}

TEST_CASE("ExtensionObject") {
    SUBCASE("Empty") {
        ExtensionObject obj;
        CHECK(obj.isEmpty());
        CHECK_FALSE(obj.isEncoded());
        CHECK_FALSE(obj.isDecoded());
        CHECK(obj.encoding() == ExtensionObjectEncoding::EncodedNoBody);
        CHECK(obj.encodedTypeId() == nullptr);
        CHECK(obj.encodedBody() == nullptr);
        CHECK(obj.decodedType() == nullptr);
        CHECK(obj.decodedData() == nullptr);
    }

    SUBCASE("fromDecoded") {
        ExtensionObject obj;
        String value("test123");
        SUBCASE("Deduce data type") {
            obj = ExtensionObject::fromDecoded(value);
        }
        SUBCASE("Custom data type") {
            obj = ExtensionObject::fromDecoded(value, UA_TYPES[UA_TYPES_STRING]);
        }
        CHECK(obj.encoding() == ExtensionObjectEncoding::DecodedNoDelete);
        CHECK(obj.isDecoded());
        CHECK(obj.decodedType() == &UA_TYPES[UA_TYPES_STRING]);
        CHECK(obj.decodedData() == value.handle());
    }

    SUBCASE("fromDecodedCopy") {
        ExtensionObject obj;
        const auto value = Variant::fromScalar(11.11);
        SUBCASE("Deduce data type") {
            obj = ExtensionObject::fromDecodedCopy(value);
        }
        SUBCASE("Custom data type") {
            obj = ExtensionObject::fromDecodedCopy(value, UA_TYPES[UA_TYPES_VARIANT]);
        }
        CHECK(obj.encoding() == ExtensionObjectEncoding::Decoded);
        CHECK(obj.isDecoded());
        CHECK(obj.decodedType() == &UA_TYPES[UA_TYPES_VARIANT]);
        CHECK(obj.decodedData<Variant>() != nullptr);
        CHECK(obj.decodedData<Variant>()->getScalar<double>() == 11.11);
    }

    SUBCASE("decodedData") {
        double value = 11.11;
        auto obj = ExtensionObject::fromDecoded(value);
        CHECK(obj.decodedData() == &value);
        CHECK(obj.decodedData<int>() == nullptr);
        CHECK(obj.decodedData<double>() == &value);
    }
}

TEST_CASE("NumericRangeDimension") {
    CHECK(NumericRangeDimension{} == NumericRangeDimension{});
    CHECK(NumericRangeDimension{1, 2} == NumericRangeDimension{1, 2});
    CHECK(NumericRangeDimension{1, 2} != NumericRangeDimension{1, 3});
}

TEST_CASE("NumericRange") {
    SUBCASE("Empty") {
        const NumericRange nr;
        CHECK(nr.empty());
        CHECK(nr.dimensions().size() == 0);
    }

    SUBCASE("From encoded range (invalid)") {
        CHECK_THROWS(NumericRange("abc"));
    }

    SUBCASE("From encoded range") {
        const NumericRange nr("1:2,0:3,5");
        CHECK(nr.dimensions().size() == 3);
        CHECK(nr.dimensions()[0] == NumericRangeDimension{1, 2});
        CHECK(nr.dimensions()[1] == NumericRangeDimension{0, 3});
        CHECK(nr.dimensions()[2] == NumericRangeDimension{5, 5});
    }

    SUBCASE("From span") {
        std::vector<NumericRangeDimension> dimensions{{1, 2}, {3, 4}};
        const NumericRange nr(dimensions);
        CHECK_FALSE(nr.empty());
        CHECK(nr.dimensions().size() == 2);
        CHECK(nr.dimensions()[0] == NumericRangeDimension{1, 2});
        CHECK(nr.dimensions()[1] == NumericRangeDimension{3, 4});
    }

    SUBCASE("From native") {
        UA_NumericRange native{};
        std::vector<NumericRangeDimension> dimensions{{1, 2}, {3, 4}};
        native.dimensionsSize = dimensions.size();
        native.dimensions = dimensions.data();
        const NumericRange nr(native);
        CHECK_FALSE(nr.empty());
        CHECK(nr.dimensions().size() == 2);
        CHECK(nr.dimensions()[0] == NumericRangeDimension{1, 2});
        CHECK(nr.dimensions()[1] == NumericRangeDimension{3, 4});
    }

    SUBCASE("Copy & move") {
        std::vector<NumericRangeDimension> dimensions{{1, 2}, {3, 4}};
        NumericRange src(dimensions);

        SUBCASE("Copy constructor") {
            const NumericRange dst(src);
            CHECK(dst.dimensions().size() == 2);
            CHECK(dst.dimensions()[0] == NumericRangeDimension{1, 2});
            CHECK(dst.dimensions()[1] == NumericRangeDimension{3, 4});
        }

        SUBCASE("Move constructor") {
            const NumericRange dst(std::move(src));
            CHECK(src.dimensions().size() == 0);
            CHECK(dst.dimensions().size() == 2);
            CHECK(dst.dimensions()[0] == NumericRangeDimension{1, 2});
            CHECK(dst.dimensions()[1] == NumericRangeDimension{3, 4});
        }

        SUBCASE("Copy assignment") {
            NumericRange dst;
            dst = src;
            CHECK(dst.dimensions().size() == 2);
            CHECK(dst.dimensions().size() == 2);
            CHECK(dst.dimensions()[0] == NumericRangeDimension{1, 2});
            CHECK(dst.dimensions()[1] == NumericRangeDimension{3, 4});
        }

        SUBCASE("Move assignment") {
            NumericRange dst;
            dst = std::move(src);
            CHECK(src.dimensions().size() == 0);
            CHECK(dst.dimensions().size() == 2);
            CHECK(dst.dimensions()[0] == NumericRangeDimension{1, 2});
            CHECK(dst.dimensions()[1] == NumericRangeDimension{3, 4});
        }
    }

    SUBCASE("toString") {
        CHECK(NumericRange({{1, 1}}).toString() == "1");
        CHECK(NumericRange({{1, 2}}).toString() == "1:2");
        CHECK(NumericRange({{1, 2}, {0, 3}, {5, 5}}).toString() == "1:2,0:3,5");
    }
}
