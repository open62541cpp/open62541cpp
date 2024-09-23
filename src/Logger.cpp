#include "open62541pp/Logger.h"

#include <cstdarg>  // va_list
#include <string>

#include "open62541pp/Client.h"
#include "open62541pp/Config.h"
#include "open62541pp/Server.h"
#include "open62541pp/detail/open62541/common.h"

namespace opcua {

static void logImpl(UA_Logger* logger, LogLevel level, LogCategory category, std::string_view msg) {
    if (logger == nullptr || logger->log == nullptr) {
        return;
    }
    va_list args{};  // NOLINT
    logger->log(
        logger->context,
        static_cast<UA_LogLevel>(level),
        static_cast<UA_LogCategory>(category),
        std::string(msg).c_str(),
        args  // NOLINT
    );
}

template <typename T>
static UA_Logger* getLogger(T* connection) noexcept {
    auto* config = detail::getConfig(connection);
    if (config == nullptr) {
        return nullptr;
    }
#if UAPP_OPEN62541_VER_GE(1, 4)
    return config->logging;
#else
    return &config->logger;
#endif
}

void log(UA_Client* client, LogLevel level, LogCategory category, std::string_view msg) {
    logImpl(getLogger(client), level, category, msg);
}

void log(Client& client, LogLevel level, LogCategory category, std::string_view msg) {
    log(client.handle(), level, category, msg);
}

void log(UA_Server* server, LogLevel level, LogCategory category, std::string_view msg) {
    logImpl(getLogger(server), level, category, msg);
}

void log(Server& server, LogLevel level, LogCategory category, std::string_view msg) {
    log(server.handle(), level, category, msg);
}

}  // namespace opcua
