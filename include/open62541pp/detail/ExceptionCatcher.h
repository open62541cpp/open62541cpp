#pragma once

#include <exception>
#include <functional>
#include <utility>  // exchange

namespace opcua {
class Client;
class Server;
}  // namespace opcua

namespace opcua::detail {
class ClientContext;
class ServerContext;
}  // namespace opcua::detail

namespace opcua::detail {

class ExceptionCatcher;

ExceptionCatcher& getExceptionCatcher(detail::ClientContext& context) noexcept;
ExceptionCatcher& getExceptionCatcher(Client& client) noexcept;
ExceptionCatcher& getExceptionCatcher(detail::ServerContext& context) noexcept;
ExceptionCatcher& getExceptionCatcher(Server& server) noexcept;

/**
 * Catch & store exceptions from user-defined callbacks in an exception-unaware context (open62541).
 * The stored exception can be rethrown in a different context.
 */
class ExceptionCatcher {
public:
    void setException(std::exception_ptr exception) noexcept {
        exception_ = std::move(exception);
    }

    bool hasException() const noexcept {
        return exception_ != nullptr;
    }

    void rethrow() {
        if (hasException()) {
            std::rethrow_exception(std::exchange(exception_, nullptr));
        }
    }

    template <typename Callback, typename... Args>
    void invoke(Callback&& callback, Args&&... args) noexcept {
        static_assert(std::is_void_v<std::invoke_result_t<Callback, Args&&...>>);
        try {
            std::invoke(std::forward<Callback>(callback), std::forward<Args>(args)...);
        } catch (...) {
            setException(std::current_exception());
        }
    }

    // template <typename OnException, typename Callback, typename... Args>
    // auto invoke(OnException&& onException, Callback&& callback, Args&&... args) {
    //     static_assert(std::is_same_v<
    //                   std::invoke_result_t<OnException, std::exception_ptr>,
    //                   std::invoke_result_t<Callback, Args&&...>>);
    //     try {
    //         return std::invoke(std::forward<Callback>(callback), std::forward<Args>(args)...);
    //     } catch (...) {
    //         setException(std::current_exception());
    //         return std::invoke(std::forward<OnException>(onException), std::current_exception());
    //     }
    // }

    template <typename Callback>
    auto wrapCallback(Callback&& callback) noexcept {
        return [this, cb = std::forward<Callback>(callback)](auto&&... args) {
            this->invoke(std::move(cb), std::forward<decltype(args)>(args)...);
        };
    }

    // template <typename OnException, typename Callback>
    // auto wrapCallback(OnException&& onException, Callback&& callback) {
    //     return [this,
    //             onException_ = std::forward<OnException>(onException),
    //             callback_ = std::forward<Callback>(callback)](auto&&... args) {
    //         return this->invoke(
    //             std::move(onException_), std::move(callback_),
    //             std::forward<decltype(args)>(args)...
    //         );
    //     };
    // }

private:
    std::exception_ptr exception_;
};

}  // namespace opcua::detail
