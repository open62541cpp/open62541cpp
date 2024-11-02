#include <string>

#include <doctest/doctest.h>

#include "open62541pp/services/detail/async_hook.hpp"
#include "open62541pp/services/detail/async_transform.hpp"

using namespace opcua;

template <typename T, typename CompletionToken>
static auto asyncTest(T result, CompletionToken&& token) {
    return asyncInitiate<T>(
        [result](auto&& handler) mutable {
            std::invoke(std::forward<decltype(handler)>(handler), result);
        },
        std::forward<CompletionToken>(token)
    );
}

TEST_CASE("TransformToken") {
    SUBCASE("Callback") {
        std::string result;
        asyncTest(
            5,
            services::detail::TransformToken(
                [](int value) { return std::to_string(value); },
                [&](const std::string& str) { result = str; }
            )
        );
        CHECK(result == "5");
    }

    SUBCASE("Future") {
        std::future<std::string> future = asyncTest(
            5,
            services::detail::TransformToken(
                [](int value) { return std::to_string(value); }, useFuture
            )
        );
        CHECK(future.get() == "5");
    }
}

TEST_CASE("HookToken") {
    bool hookExecuted = false;
    int hookResult = 0;
    int result = 0;
    asyncTest(
        5,
        services::detail::HookToken(
            [&](const int& value) {
                hookExecuted = true;
                hookResult = value;
            },
            [&](int& value) { result = value; }
        )
    );
    CHECK(hookExecuted);
    CHECK(hookResult == 5);
    CHECK(result == 5);
}
