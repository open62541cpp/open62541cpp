#include <chrono>
#include <thread>

#include <doctest/doctest.h>

#include "open62541pp/Client.h"
#include "open62541pp/MonitoredItem.h"
#include "open62541pp/Server.h"
#include "open62541pp/Subscription.h"

#include "helper/Runner.h"

using namespace opcua;
using namespace std::literals::chrono_literals;

#ifdef UA_ENABLE_SUBSCRIPTIONS
TEST_CASE("Subscription & MonitoredItem (server)") {
    Server server;

    auto sub = server.createSubscription();

    CHECK(sub.getConnection() == server);
    CHECK(sub.getMonitoredItems().empty());

    MonitoringParameters monitoringParameters{};
    monitoringParameters.samplingInterval = 0.0;  // = fastest practical

    size_t notificationCount = 0;
    auto mon = sub.subscribeDataChange(
        VariableId::Server_ServerStatus_CurrentTime,
        AttributeId::Value,
        MonitoringMode::Reporting,
        monitoringParameters,
        [&](const auto& item, const DataValue&) {
            CHECK(item.getNodeId() == NodeId(VariableId::Server_ServerStatus_CurrentTime));
            CHECK(item.getAttributeId() == AttributeId::Value);
            notificationCount++;
        }
    );
    CHECK(sub.getMonitoredItems().size() == 1);

    std::this_thread::sleep_for(100ms);
    server.runIterate();
    CHECK(notificationCount > 0);

    mon.deleteMonitoredItem();
    CHECK(sub.getMonitoredItems().empty());
}

TEST_CASE("Subscription & MonitoredItem (client)") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;
    client.connect("opc.tcp://localhost:4840");

    SUBCASE("Create & delete subscription") {
        CHECK(client.getSubscriptions().empty());

        SubscriptionParameters parameters{};
        auto sub = client.createSubscription(parameters);
        CAPTURE(sub.getSubscriptionId());

        CHECK(client.getSubscriptions().size() == 1);
        CHECK(client.getSubscriptions().at(0) == sub);

        CHECK(sub.getConnection() == client);
        CHECK(sub.getMonitoredItems().empty());

        sub.deleteSubscription();
        CHECK(client.getSubscriptions().empty());
        CHECK_THROWS_WITH(sub.deleteSubscription(), "BadSubscriptionIdInvalid");
    }

    SUBCASE("Modify subscription") {
        auto sub = client.createSubscription();
        sub.setPublishingMode(false);

        SubscriptionParameters parameters{};
        parameters.priority = 10;
        sub.setSubscriptionParameters(parameters);

        CHECK(parameters.priority == 10);  // not revised by server
    }

    SUBCASE("Monitor data change") {
        SubscriptionParameters subscriptionParameters{};
        MonitoringParameters monitoringParameters{};

        auto sub = client.createSubscription(subscriptionParameters);
        sub.setPublishingMode(false);  // enable later

        size_t notificationCount = 0;
        auto mon = sub.subscribeDataChange(
            VariableId::Server_ServerStatus_CurrentTime,
            AttributeId::Value,
            MonitoringMode::Sampling,  // won't trigger notifications
            monitoringParameters,
            [&](const auto& item, const DataValue&) {
                CHECK(item.getNodeId() == NodeId(VariableId::Server_ServerStatus_CurrentTime));
                CHECK(item.getAttributeId() == AttributeId::Value);
                notificationCount++;
            }
        );

        CHECK(sub.getMonitoredItems().size() == 1);
        CHECK(sub.getMonitoredItems().at(0) == mon);

        client.runIterate();
        CHECK(notificationCount == 0);

        sub.setPublishingMode(true);  // monitoring mode is still sampling -> no notifications
        client.runIterate();
        CHECK(notificationCount == 0);

        mon.setMonitoringMode(MonitoringMode::Reporting);  // now we should get a notification
        client.runIterate();
        CHECK(notificationCount > 0);

        mon.deleteMonitoredItem();
        CHECK_THROWS_WITH(mon.deleteMonitoredItem(), "BadMonitoredItemIdInvalid");
    }

    SUBCASE("Modify monitored item") {
        auto sub = client.createSubscription();
        auto mon = sub.subscribeDataChange(
            VariableId::Server_ServerStatus_CurrentTime, AttributeId::Value, {}
        );

        mon.setMonitoringMode(MonitoringMode::Disabled);

        MonitoringParameters monitoringParameters{};
        monitoringParameters.samplingInterval = 0.0;  // = fastest practical rate
        mon.setMonitoringParameters(monitoringParameters);
        CHECK(monitoringParameters.samplingInterval > 0.0);
    }

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
    SUBCASE("Monitor event") {
        auto sub = client.createSubscription();

        MonitoringParameters monitoringParameters{};
        auto mon = sub.subscribeEvent(
            ObjectId::Server,
            MonitoringMode::Reporting,
            monitoringParameters,
            [](const auto& item, const auto& eventFields) {
                CAPTURE(item.getSubscriptionId());
                CAPTURE(item.getMonitoredItemId());
                CAPTURE(eventFields.size());
            }
        );

        CHECK(sub.getMonitoredItems().size() == 1);
        CHECK(sub.getMonitoredItems().at(0) == mon);
    }
#endif
}
#endif
