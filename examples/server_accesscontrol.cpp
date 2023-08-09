#include <iostream>

#include "open62541pp/open62541pp.h"

using namespace opcua;

// Custom access control based on AccessControlDefault.
// If a user logs in with the username "admin", a session attribute "isAdmin" is stored. As an
// example, the user "admin" has write access level to the created variable node. So admins can
// change the value of the created variable node, anonymous users and the user "user" can't.
class AccessControlCustom : public AccessControlDefault {
public:
    using Base = AccessControlDefault;
    using Base::Base;  // inherit constructors

    StatusCode activateSession(
        Session& session,
        const EndpointDescription& endpointDescription,
        const ByteString& secureChannelRemoteCertificate,
        const ExtensionObject& userIdentityToken
    ) override {
        // Grant admin rights if user is logged in as "admin"
        // Store attribute "isAdmin" as session attribute to use it in access callbacks
        const auto* token = userIdentityToken.getDecodedData<UserNameIdentityToken>();
        const bool isAdmin = (token != nullptr && token->getUserName() == "admin");
        std::cout << "User has admin rights: " << isAdmin << std::endl;
        session.setSessionAttribute({0, "isAdmin"}, Variant::fromScalar(isAdmin));

        return Base::activateSession(
            session, endpointDescription, secureChannelRemoteCertificate, userIdentityToken
        );
    }

    uint8_t getUserAccessLevel(Session& session, const NodeId& nodeId) override {
        const bool isAdmin = session.getSessionAttribute({0, "isAdmin"}).getScalar<bool>();
        std::cout << "Get user access level of node id " << nodeId.toString() << std::endl;
        std::cout << "Admin rights granted: " << isAdmin << std::endl;
        return isAdmin ? UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE
                       : UA_ACCESSLEVELMASK_READ;
    }
};

int main() {
    // Exchanging usernames/passwords without encryption as plain text is dangerous.
    // We are doing this just for demonstration, don't use it in production!
    Server server;

    AccessControlCustom accessControl(
        true,  // allow anonymous
        {
            {"admin", "admin"},
            {"user", "user"},
        }
    );
    server.setAccessControl(accessControl);

    // Add variable node. Try to change its value as a client with different logins.
    server.getObjectsNode().addVariable(
        {1, 1000},
        "Variable",
        VariableAttributes{}
            .setAccessLevel(UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE)
            .setDataType(DataTypeId::Int32)
            .setValueRank(ValueRank::Scalar)
            .setValueScalar(0)
    );

    server.run();
}
