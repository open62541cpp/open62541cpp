#include "open62541pp/plugin/log.hpp"

#include <cassert>
#include <cstdarg>  // va_list

#include "open62541pp/config.hpp"
#include "open62541pp/detail/string_utils.hpp"  // detail::toString

namespace opcua {

static void logNative(
    void* context, UA_LogLevel level, UA_LogCategory category, const char* msg, va_list args
) {
    assert(context != nullptr);
    static_cast<LoggerBase*>(context)->log(
        static_cast<LogLevel>(level),
        static_cast<LogCategory>(category),
        detail::toString(msg, args)
    );
}

UA_Logger LoggerBase::create(bool ownsAdapter) {
    UA_Logger native{};
    native.log = logNative;
    native.context = this;
#if UAPP_OPEN62541_VER_GE(1, 4)
    if (ownsAdapter) {
        native.clear = [](UA_Logger* logger) {
            if (logger != nullptr) {
                delete static_cast<LoggerBase*>(logger->context);  // NOLINT
                logger->context = nullptr;
                UA_free(logger);
                logger = nullptr;
            }
        };
    } else {
        native.clear = [](UA_Logger* logger) {
            UA_free(logger);
            logger = nullptr;
        };
    }
#else
    if (ownsAdapter) {
        native.clear = [](void* context) {
            delete static_cast<LoggerBase*>(context);  // NOLINT
            context = nullptr;
        };
    }
#endif
    return native;
}

void LoggerBase::clear(UA_Logger& native) const noexcept {
    if (native.clear != nullptr) {
#if UAPP_OPEN62541_VER_GE(1, 4)
        // TODO:
        // Open62541 v1.4 transitioned to pointers for UA_Logger instances.
        // The clear function doesn't clear the context anymore but frees the memory and
        // consequently invalidates pointers like UA_EventLoop.logger.
        // Neighter the open62541 loggers UA_Log_Syslog_log, UA_Log_Syslog_log, nor the
        // opcua::Logger needs to be cleared, so skip this for now.

        // native.clear(&native);
#else
        native.clear(native.context);
        native.context = nullptr;
#endif
    }
}

// void LoggerBase::clear(UA_Logger*& plugin) const noexcept override {
//     if (plugin != nullptr) {
//         clear(*plugin);
//         plugin = nullptr;
//     }
// }

}  // namespace opcua
