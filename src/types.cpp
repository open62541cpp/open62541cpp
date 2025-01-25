#include "open62541pp/types.hpp"

#include <ctime>  // gmtime, localtime
#include <iomanip>  // put_time
#include <ostream>
#include <sstream>

#include "open62541pp/config.hpp"

namespace opcua {

/* ------------------------------------------- String ------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const String& str) {
    os << static_cast<std::string_view>(str);
    return os;
}

/* -------------------------------------------- Guid -------------------------------------------- */

std::string Guid::toString() const {
    return std::string(opcua::toString(*this));
}

std::ostream& operator<<(std::ostream& os, const Guid& guid) {
    os << toString(guid);
    return os;
}

/* ------------------------------------------ DateTime ------------------------------------------ */

std::string DateTime::format(std::string_view format, bool localtime) const {
    const std::time_t unixTime = toUnixTime();
    std::ostringstream ss;
    const auto* timeinfo = localtime ? std::localtime(&unixTime) : std::gmtime(&unixTime);
    if (timeinfo != nullptr) {
        ss << std::put_time(timeinfo, std::string(format).c_str());
    }
    return ss.str();
}

/* ----------------------------------------- ByteString ----------------------------------------- */

ByteString ByteString::fromBase64([[maybe_unused]] std::string_view encoded) {
#if UAPP_OPEN62541_VER_GE(1, 1)
    ByteString output;
    UA_ByteString_fromBase64(output.handle(), String(encoded).handle());
    return output;
#else
    return {};
#endif
}

#if UAPP_OPEN62541_VER_GE(1, 1)
String ByteString::toBase64() const {
    String output;
    UA_ByteString_toBase64(handle(), output.handle());
    return output;
}
#endif

/* ----------------------------------------- XmlElement ----------------------------------------- */

std::ostream& operator<<(std::ostream& os, const XmlElement& xmlElement) {
    os << static_cast<std::string_view>(xmlElement);
    return os;
}

/* ------------------------------------------- NodeId ------------------------------------------- */

std::string NodeId::toString() const {
    return std::string{opcua::toString(*this)};
}

/* --------------------------------------- ExpandedNodeId --------------------------------------- */

std::string ExpandedNodeId::toString() const {
    return std::string{opcua::toString(*this)};
}

/* ---------------------------------------- NumericRange ---------------------------------------- */

NumericRange::NumericRange(std::string_view encodedRange) {
    const auto encodedRangeNative = detail::toNativeString(encodedRange);
#if UAPP_OPEN62541_VER_GE(1, 1)
    throwIfBad(UA_NumericRange_parse(handle(), encodedRangeNative));
#else
    throwIfBad(UA_NumericRange_parseFromString(handle(), &encodedRangeNative));
#endif
}

static std::string toStringImpl(const NumericRange& range) {
    std::ostringstream ss;
    for (const auto& dimension : range.dimensions()) {
        ss << dimension.min;
        if (dimension.min != dimension.max) {
            ss << ':' << dimension.max;
        }
        ss << ',';
    }
    auto str = ss.str();
    str.pop_back();  // remove last comma
    return str;
}

std::string NumericRange::toString() const {
    return toStringImpl(*this);
}

String toString(const NumericRange& range) {
    return String(toStringImpl(range));
}

}  // namespace opcua
