#include <iostream>

#include <open62541pp/node.hpp>
#include <open62541pp/plugin/accesscontrol_default.hpp>
#include <open62541pp/server.hpp>

using namespace opcua;

// Custom access control based on AccessControlDefault.
// If a user logs in with the username "admin", a session attribute "isAdmin" is stored. As an
// example, the user "admin" has write access level to the created variable node. So admins can
// change the value of the created variable node, anonymous users and the user "user" can't.
// Session attributes are available since open62541 v1.3, so this example requires at least v1.3.
class AccessControlCustom : public AccessControlDefault {
public:
    using AccessControlDefault::AccessControlDefault;  // inherit constructors

    StatusCode activateSession(
        Session& session,
        const EndpointDescription& endpointDescription,
        const ByteString& secureChannelRemoteCertificate,
        const ExtensionObject& userIdentityToken
    ) override {
        // Grant admin rights if user is logged in as "admin"
        // Store attribute "isAdmin" as session attribute to use it in access callbacks
        const auto* token = userIdentityToken.decodedData<UserNameIdentityToken>();
        const bool isAdmin = (token != nullptr && token->userName() == "admin");
        std::cout << "User has admin rights: " << isAdmin << std::endl;
        session.setSessionAttribute({0, "isAdmin"}, Variant(isAdmin));

        return AccessControlDefault::activateSession(
            session, endpointDescription, secureChannelRemoteCertificate, userIdentityToken
        );
    }

    Bitmask<AccessLevel> getUserAccessLevel(Session& session, const NodeId& nodeId) override {
        const bool isAdmin = session.getSessionAttribute({0, "isAdmin"}).scalar<bool>();
        std::cout << "Get user access level of node id " << opcua::toString(nodeId) << std::endl;
        std::cout << "Admin rights granted: " << isAdmin << std::endl;
        return isAdmin
            ? AccessLevel::CurrentRead | AccessLevel::CurrentWrite
            : AccessLevel::CurrentRead;
    }
};

int main() {
    // Exchanging usernames/passwords without encryption as plain text is dangerous.
    // We are doing this just for demonstration, don't use it in production!
    AccessControlCustom accessControl(
        true,  // allow anonymous
        {
            {"admin", "admin"},
            {"user", "user"},
        }
    );

    ServerConfig config;
    config.setAccessControl(accessControl);

    Server server(std::move(config));

    // Add variable node. Try to change its value as a client with different logins.
    Node(server, ObjectId::ObjectsFolder)
        .addVariable(
            {1, 1000},
            "Variable",
            VariableAttributes{}
                .setAccessLevel(AccessLevel::CurrentRead | AccessLevel::CurrentWrite)
                .setDataType(DataTypeId::Int32)
                .setValueRank(ValueRank::Scalar)
                .setValueScalar(0)
        );

    server.run();
}
