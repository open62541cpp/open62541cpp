#include <iostream>

#include <open62541pp/client.hpp>
#include <open62541pp/services/attribute_highlevel.hpp>
#include <open62541pp/services/monitoreditem.hpp>
#include <open62541pp/services/subscription.hpp>
#include <open62541pp/services/view.hpp>

int main() {
    opcua::Client client;

    client.onConnected([] { std::cout << "Client connected" << std::endl; });
    client.onSessionActivated([&] {
        std::cout << "Session activated" << std::endl;

        // Schedule async operations once the client session is activated
        // 1. Read request
        opcua::services::readValueAsync(
            client,
            opcua::VariableId::Server_ServerStatus_CurrentTime,
            [](opcua::Result<opcua::Variant>& result) {
                std::cout << "Read result with status code: " << result.code() << std::endl;
                const auto dt = result.value().scalar<opcua::DateTime>();
                std::cout << "Server date (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;
            }
        );

        // 2. Browse request of Server object
        const opcua::BrowseDescription bd(
            opcua::ObjectId::Server,
            opcua::BrowseDirection::Forward,
            opcua::ReferenceTypeId::References
        );
        opcua::services::browseAsync(client, bd, 0, [](opcua::BrowseResult& result) {
            std::cout << "Browse result with " << result.references().size() << " references:\n";
            for (const auto& reference : result.references()) {
                std::cout << "- " << reference.browseName().name() << std::endl;
            }
        });

        // 3. Subscription
        opcua::services::createSubscriptionAsync(
            client,
            opcua::SubscriptionParameters{},  // default subscription parameters
            true,  // publishingEnabled
            {},  // statusChangeCallback
            [](opcua::IntegerId subId) {
                std::cout << "Subscription deleted: " << subId << std::endl;
            },
            [&](opcua::CreateSubscriptionResponse& response) {
                std::cout
                    << "Subscription created:\n"
                    << "- status code: " << response.responseHeader().serviceResult() << "\n"
                    << "- subscription id: " << response.subscriptionId() << std::endl;

                // Create MonitoredItem
                opcua::services::createMonitoredItemDataChangeAsync(
                    client,
                    response.subscriptionId(),
                    opcua::ReadValueId(
                        opcua::VariableId::Server_ServerStatus_CurrentTime,
                        opcua::AttributeId::Value
                    ),
                    opcua::MonitoringMode::Reporting,
                    opcua::MonitoringParametersEx{},  // default monitoring parameters
                    [](opcua::IntegerId subId, opcua::IntegerId monId, const opcua::DataValue& dv) {
                        std::cout
                            << "Data change notification:\n"
                            << "- subscription id: " << subId << "\n"
                            << "- monitored item id: " << monId << "\n"
                            << "- value: " << opcua::toString(dv) << std::endl;
                    },
                    {},  // delete callback
                    [](opcua::MonitoredItemCreateResult& result) {
                        std::cout
                            << "MonitoredItem created:\n"
                            << "- status code: " << result.statusCode() << "\n"
                            << "- monitored item id: " << result.monitoredItemId() << std::endl;
                    }
                );
            }
        );
    });
    client.onSessionClosed([] { std::cout << "Session closed" << std::endl; });
    client.onDisconnected([] { std::cout << "Client disconnected" << std::endl; });

    client.connectAsync("opc.tcp://localhost:4840");
    client.run();
}
