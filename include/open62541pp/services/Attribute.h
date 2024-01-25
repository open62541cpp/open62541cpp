#pragma once

#include <cstdint>
#include <vector>

#include "open62541pp/Bitmask.h"
#include "open62541pp/Common.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

// forward declare
namespace opcua {
class Client;
}

namespace opcua::services {

/* -------------------------------------- Generic functions ------------------------------------- */

/**
 * @defgroup Attribute Attribute service set
 * Read and write node attributes.
 *
 * The following node attributes cannot be changed once a node has been created:
 * - NodeClass
 * - NodeId
 * - Symmetric
 * - ContainsNoLoops
 *
 * The following attributes cannot be written from the server, as they are specific to the different
 * users and set by the access control callback:
 * - UserWriteMask
 * - UserAccessLevel
 * - UserExecutable
 *
 * @see https://www.open62541.org/doc/1.3/server.html
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10
 * @ingroup Services
 * @{
 */

/**
 * Generic function to read one or more attributes of one or more nodes (client only).
 */
ReadResponse read(Client& client, const ReadRequest& request);

/**
 * @overload
 */
ReadResponse read(
    Client& client,
    Span<const ReadValueId> nodesToRead,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither
);

/**
 * Generic function to read node attributes.
 */
template <typename T>
DataValue readAttribute(
    T& serverOrClient,
    const NodeId& id,
    AttributeId attributeId,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither
);

/// Helper function to read scalar node attributes.
template <typename AttributeType, typename T>
inline auto readAttributeScalar(T& serverOrClient, const NodeId& id, AttributeId attributeId) {
    const auto dv = readAttribute(serverOrClient, id, attributeId);
    return dv.getValue().template getScalarCopy<AttributeType>();
}

/**
 * Generic function to write one or more attributes of one or more nodes (client only).
 */
WriteResponse write(Client& client, const WriteRequest& request);

/**
 * @overload
 */
WriteResponse write(Client& client, Span<const WriteValue> nodesToWrite);

/**
 * Generic function to write node attributes.
 */
template <typename T>
void writeAttribute(
    T& serverOrClient, const NodeId& id, AttributeId attributeId, const DataValue& value
);

/* -------------------------------- Specialized inline functions -------------------------------- */

/**
 * Read the `NodeId` attribute of a node.
 *
 * This function is mainly used to check the existence of a node.
 */
template <typename T>
inline NodeId readNodeId(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<NodeId>(serverOrClient, id, AttributeId::NodeId);
}

/**
 * Read the `NodeClass` attribute of a node.
 */
template <typename T>
inline NodeClass readNodeClass(T& serverOrClient, const NodeId& id) {
    auto dv = readAttribute(serverOrClient, id, AttributeId::NodeClass);
    // workaround to read enum from variant...
    return *static_cast<NodeClass*>(dv.getValue().handle()->data);
}

/**
 * Read the `BrowseName` attribute of a node.
 *
 * A non-localised human-readable name used to browse the address space.
 * The browse name of a reference type must be unique in a server.
 */
template <typename T>
inline QualifiedName readBrowseName(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<QualifiedName>(serverOrClient, id, AttributeId::BrowseName);
}

/**
 * Read the `DisplayName` attribute of a node.
 *
 * Used to display the node to the user.
 */
template <typename T>
inline LocalizedText readDisplayName(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<LocalizedText>(serverOrClient, id, AttributeId::DisplayName);
}

/**
 * Read the `Description` attribute of a node.
 *
 * The description shall explain the meaning of the node.
 */
template <typename T>
inline LocalizedText readDescription(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<LocalizedText>(serverOrClient, id, AttributeId::Description);
}

/**
 * Read the `WriteMask` attribute of a node.
 */
template <typename T>
inline Bitmask<WriteMask> readWriteMask(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<uint32_t>(serverOrClient, id, AttributeId::WriteMask);
}

/**
 * Read the `UserWriteMask` attribute of a node.
 *
 * In contrast to the write mask, the user write mask is taking access rights into account.
 */
template <typename T>
inline Bitmask<WriteMask> readUserWriteMask(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<uint32_t>(serverOrClient, id, AttributeId::UserWriteMask);
}

/**
 * Read the `IsAbtract` attribute of a reference type node.
 *
 * If a reference is abstract, no reference of this type shall exist, only of its subtypes.
 */
template <typename T>
inline bool readIsAbstract(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<bool>(serverOrClient, id, AttributeId::IsAbstract);
}

/**
 * Read the `Symmetric` attribute of a reference type node.
 *
 * If a reference is symmetric it can seen from both the source and target node.
 */
template <typename T>
inline bool readSymmetric(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<bool>(serverOrClient, id, AttributeId::Symmetric);
}

/**
 * Read the `InverseName` attribute of a reference type node.
 *
 * The inverse name describes the reference type as seen from the target node.
 * For example, the reference type `HasSubtype` has the inverse name `HasSupertype`.
 */
template <typename T>
inline LocalizedText readInverseName(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<LocalizedText>(serverOrClient, id, AttributeId::InverseName);
}

/**
 * Read the `ContainsNoLoops` attribute of a view node.
 */
template <typename T>
inline bool readContainsNoLoops(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<bool>(serverOrClient, id, AttributeId::ContainsNoLoops);
}

/**
 * Read the `EventNotifier` attribute of an object or view node.
 */
template <typename T>
inline Bitmask<EventNotifier> readEventNotifier(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<uint8_t>(serverOrClient, id, AttributeId::EventNotifier);
}

/**
 * Read the `Value` attribute of a variable node as a DataValue object.
 */
template <typename T>
inline DataValue readDataValue(T& serverOrClient, const NodeId& id) {
    return readAttribute(serverOrClient, id, AttributeId::Value, TimestampsToReturn::Both);
}

/**
 * Read the `Value` attribute of a variable node as a Variant object.
 */
template <typename T>
inline Variant readValue(T& serverOrClient, const NodeId& id) {
    DataValue dv = readAttribute(serverOrClient, id, AttributeId::Value);
    return std::move(dv.getValue());
}

/**
 * Read the `DataType` attribute of a variable (type) node as NodeId.
 */
template <typename T>
inline NodeId readDataType(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<NodeId>(serverOrClient, id, AttributeId::DataType);
}

/**
 * Read the `ValueRank` attribute of a variable (type) node.
 *
 * Indicates whether the value attribute of the variable node is an array and how many dimensions
 * the array has.
 */
template <typename T>
inline ValueRank readValueRank(T& serverOrClient, const NodeId& id) {
    const auto index = readAttributeScalar<int32_t>(serverOrClient, id, AttributeId::ValueRank);
    return static_cast<ValueRank>(index);
}

/**
 * Read the `ArrayDimensions` attribute of a variable (type) node.
 *
 * Specifies the maximum supported length of each dimension (0 if unknown).
 */
template <typename T>
inline std::vector<uint32_t> readArrayDimensions(T& serverOrClient, const NodeId& id) {
    const auto dv = readAttribute(serverOrClient, id, AttributeId::ArrayDimensions);
    if (dv.getValue().isArray()) {
        return dv.getValue().template getArrayCopy<uint32_t>();
    }
    return {};
}

/**
 * Read the `AccessLevel` attribute of a variable node.
 *
 * The access level attribute is used to indicate how the value of a variable can be accessed
 * (read/write) and if it contains current and/or historic data.
 */
template <typename T>
inline Bitmask<AccessLevel> readAccessLevel(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<uint8_t>(serverOrClient, id, AttributeId::AccessLevel);
}

/**
 * Read the `UserAccessLevel` attribute a variable node.
 *
 * @copydetails readAccessLevel
 * In contrast to the access level, the user access level is taking access rights into account.
 */
template <typename T>
inline Bitmask<AccessLevel> readUserAccessLevel(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<uint8_t>(serverOrClient, id, AttributeId::UserAccessLevel);
}

/**
 * Read the `MinimumSamplingInterval` attribute of a variable node.
 *
 * Specifies (in milliseconds) how fast the server can reasonably sample the value for changes.
 * A minimum sampling interval of 0 indicates that the server is to monitor the item continuously.
 * A minimum sampling interval of -1 means indeterminate.
 */
template <typename T>
inline double readMinimumSamplingInterval(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<double>(serverOrClient, id, AttributeId::MinimumSamplingInterval);
}

/**
 * Read the `Historizing` attribute of a variable node.
 */
template <typename T>
inline bool readHistorizing(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<bool>(serverOrClient, id, AttributeId::Historizing);
}

/**
 * Read the `Executable` attribute of a method node.
 */
template <typename T>
inline bool readExecutable(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<bool>(serverOrClient, id, AttributeId::Executable);
}

/**
 * Read the `UserExecutable` attribute of a method node.
 */
template <typename T>
inline bool readUserExecutable(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<bool>(serverOrClient, id, AttributeId::UserExecutable);
}

/* ---------------------------------------------------------------------------------------------- */

/**
 * Write the `DisplayName` attribute of a node.
 * @copydetails readDisplayName
 */
template <typename T>
inline void writeDisplayName(T& serverOrClient, const NodeId& id, const LocalizedText& name) {
    writeAttribute(serverOrClient, id, AttributeId::DisplayName, DataValue::fromScalar(name));
}

/**
 * Write the `Description` attribute of a node.
 * @copydetails readDescription
 */
template <typename T>
inline void writeDescription(T& serverOrClient, const NodeId& id, const LocalizedText& desc) {
    writeAttribute(serverOrClient, id, AttributeId::Description, DataValue::fromScalar(desc));
}

/**
 * Write the `WriteMask` attribute of a node.
 * @copydetails readWriteMask
 */
template <typename T>
inline void writeWriteMask(T& serverOrClient, const NodeId& id, Bitmask<WriteMask> mask) {
    writeAttribute(serverOrClient, id, AttributeId::WriteMask, DataValue::fromScalar(mask.get()));
}

/**
 * Write the `UserWriteMask` attribute of a node.
 * @copydetails readUserWriteMask
 * @note Cannot be written from the server.
 */
template <typename T>
inline void writeUserWriteMask(T& serverOrClient, const NodeId& id, Bitmask<WriteMask> mask) {
    writeAttribute(
        serverOrClient, id, AttributeId::UserWriteMask, DataValue::fromScalar(mask.get())
    );
}

/**
 * Write the `IsAbstract` attribute of a reference type node.
 * @copydetails readIsAbstract
 */
template <typename T>
inline void writeIsAbstract(T& serverOrClient, const NodeId& id, bool isAbstract) {
    writeAttribute(serverOrClient, id, AttributeId::IsAbstract, DataValue::fromScalar(isAbstract));
}

/**
 * Write the `Symmetric` attribute of a reference type node.
 * @copydetails readSymmetric
 */
template <typename T>
inline void writeSymmetric(T& serverOrClient, const NodeId& id, bool symmetric) {
    writeAttribute(serverOrClient, id, AttributeId::Symmetric, DataValue::fromScalar(symmetric));
}

/**
 * Write the `InverseName` of a reference type node.
 * @copydetails readInverseName
 */
template <typename T>
inline void writeInverseName(T& serverOrClient, const NodeId& id, const LocalizedText& name) {
    writeAttribute(serverOrClient, id, AttributeId::InverseName, DataValue::fromScalar(name));
}

/**
 * Write the `ContainsNoLoops` attribute of a view node.
 * @copydetails readContainsNoLoops
 */
template <typename T>
inline void writeContainsNoLoops(T& serverOrClient, const NodeId& id, bool containsNoLoops) {
    writeAttribute(
        serverOrClient, id, AttributeId::ContainsNoLoops, DataValue::fromScalar(containsNoLoops)
    );
}

/**
 * Write the `EventNotifier` attribute of an object or view node.
 * @copydetails readEventNotifier
 */
template <typename T>
inline void writeEventNotifier(T& serverOrClient, const NodeId& id, Bitmask<EventNotifier> mask) {
    writeAttribute(
        serverOrClient, id, AttributeId::EventNotifier, DataValue::fromScalar(mask.get())
    );
}

/**
 * Write the `Value` attribute of a variable node as a DataValue object.
 * @copydetails readDataValue
 */
template <typename T>
inline void writeDataValue(T& serverOrClient, const NodeId& id, const DataValue& value) {
    writeAttribute(serverOrClient, id, AttributeId::Value, value);
}

/**
 * Write the `Value` attribute of a variable node as a Variant object.
 * @copydetails readValue
 */
template <typename T>
inline void writeValue(T& serverOrClient, const NodeId& id, const Variant& value) {
    UA_DataValue dv{};
    dv.value = *value.handle();  // shallow copy
    dv.hasValue = true;
    writeAttribute(serverOrClient, id, AttributeId::Value, asWrapper<DataValue>(dv));
}

/**
 * Write the `DataType` attribute of a variable (type) node.
 * @copydetails readDataType
 */
template <typename T>
inline void writeDataType(T& serverOrClient, const NodeId& id, const NodeId& typeId) {
    writeAttribute(serverOrClient, id, AttributeId::DataType, DataValue::fromScalar(typeId));
}

/**
 * Write the `ValueRank` attribute of a variable (type) node.
 * @copydetails readValueRank
 */
template <typename T>
inline void writeValueRank(T& serverOrClient, const NodeId& id, ValueRank valueRank) {
    const auto native = static_cast<int32_t>(valueRank);
    writeAttribute(serverOrClient, id, AttributeId::ValueRank, DataValue::fromScalar(native));
}

/**
 * Write the `ArrayDimensions` attribute of a variable (type) node.
 *
 * Should be unspecified if ValueRank is <= 0 (ValueRank::Any, ValueRank::Scalar,
 * ValueRank::ScalarOrOneDimension, ValueRank::OneOrMoreDimensions). The dimension zero is a
 * wildcard and the actual value may have any length in this dimension.
 * @copydetails readArrayDimensions
 */
template <typename T>
inline void writeArrayDimensions(
    T& serverOrClient, const NodeId& id, Span<const uint32_t> dimensions
) {
    writeAttribute(
        serverOrClient, id, AttributeId::ArrayDimensions, DataValue::fromArray(dimensions)
    );
}

/**
 * Write the `AccessLevel` attribute of a variable node.
 * @copydetails readAccessLevel
 */
template <typename T>
inline void writeAccessLevel(T& serverOrClient, const NodeId& id, Bitmask<AccessLevel> mask) {
    writeAttribute(serverOrClient, id, AttributeId::AccessLevel, DataValue::fromScalar(mask.get()));
}

/**
 * Write the `UserAccessLevel` attribute of a variable node.
 * @copydetails readUserAccessLevel
 * @note Cannot be written from the server.
 */
template <typename T>
inline void writeUserAccessLevel(T& serverOrClient, const NodeId& id, Bitmask<AccessLevel> mask) {
    writeAttribute(
        serverOrClient, id, AttributeId::UserAccessLevel, DataValue::fromScalar(mask.get())
    );
}

/**
 * Write the `MinimumSamplingInterval` attribute of a variable node.
 * @copydetails readMinimumSamplingInterval
 */
template <typename T>
inline void writeMinimumSamplingInterval(T& serverOrClient, const NodeId& id, double milliseconds) {
    writeAttribute(
        serverOrClient,
        id,
        AttributeId::MinimumSamplingInterval,
        DataValue::fromScalar(milliseconds)
    );
}

/**
 * Write the `Historizing` attribute of a variable node.
 * @copydetails readHistorizing
 */
template <typename T>
inline void writeHistorizing(T& serverOrClient, const NodeId& id, bool historizing) {
    writeAttribute(
        serverOrClient, id, AttributeId::Historizing, DataValue::fromScalar(historizing)
    );
}

/**
 * Write the `Executable` attribute of a method node.
 * @copydetails readExecutable
 */
template <typename T>
inline void writeExecutable(T& serverOrClient, const NodeId& id, bool executable) {
    writeAttribute(serverOrClient, id, AttributeId::Executable, DataValue::fromScalar(executable));
}

/**
 * Write the `UserExecutable` attribute of a method node.
 * @copydetails readUserExecutable
 */
template <typename T>
inline void writeUserExecutable(T& serverOrClient, const NodeId& id, bool userExecutable) {
    writeAttribute(
        serverOrClient, id, AttributeId::UserExecutable, DataValue::fromScalar(userExecutable)
    );
}

/**
 * @}
 */

}  // namespace opcua::services
