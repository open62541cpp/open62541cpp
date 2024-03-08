#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>  // exchange, swap

#include "open62541pp/Common.h"  // TypeIndex
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Wrapper.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/detail/open62541/common.h"

namespace opcua {

/**
 * Template base class to wrap `UA_*` type objects.
 *
 * Zero cost abstraction to wrap the C API objects and delete them on destruction. The derived
 * classes should implement specific constructors to convert from other data types.
 *
 * @warning No virtual constructor defined, don't implement a destructor in the derived classes.
 * @ingroup Wrapper
 */
template <typename T, TypeIndex typeIndex>
class TypeWrapper : public Wrapper<T> {
public:
    static_assert(typeIndex < UA_TYPES_COUNT);

    using TypeWrapperBase = TypeWrapper<T, typeIndex>;

    constexpr TypeWrapper() = default;

    /// Constructor with native object (deep copy).
    explicit constexpr TypeWrapper(const T& native)
        : Wrapper<T>(detail::copy(native, UA_TYPES[typeIndex])) {}

    /// Constructor with native object (move rvalue).
    constexpr TypeWrapper(T&& native) noexcept  // NOLINT, implicit wanted
        : Wrapper<T>(std::exchange(native, {})) {}

    ~TypeWrapper() {  // NOLINT
        clear();
    };

    /// Copy constructor (deep copy).
    constexpr TypeWrapper(const TypeWrapper& other)
        : Wrapper<T>(detail::copy(other.native(), UA_TYPES[typeIndex])) {}

    /// Move constructor.
    constexpr TypeWrapper(TypeWrapper&& other) noexcept
        : Wrapper<T>(std::exchange(other.native(), {})) {}

    /// Copy assignment (deep copy).
    constexpr TypeWrapper& operator=(const TypeWrapper& other) {  // NOLINT, false positive
        if (this != &other) {
            clear();
            this->native() = detail::copy(other.native(), UA_TYPES[typeIndex]);
        }
        return *this;
    }

    /// Copy assignment with native object (deep copy).
    constexpr TypeWrapper& operator=(const T& native) {
        if (&this->native() != &native) {
            clear();
            this->native() = detail::copy(native, UA_TYPES[typeIndex]);
        }
        return *this;
    }

    /// Move assignment.
    constexpr TypeWrapper& operator=(TypeWrapper&& other) noexcept {
        if (this != &other) {
            clear();
            this->native() = std::exchange(other.native(), {});
        }
        return *this;
    }

    /// Move assignment with native object.
    constexpr TypeWrapper& operator=(T&& native) noexcept {
        if (&this->native() != &native) {
            clear();
            this->native() = std::exchange(native, {});
        }
        return *this;
    }

    /// Swap with wrapper object.
    constexpr void swap(TypeWrapper& other) noexcept {
        static_assert(std::is_nothrow_swappable_v<T>);
        std::swap(this->native(), other.native());
    }

    /// Swap with native object.
    constexpr void swap(T& native) noexcept {
        static_assert(std::is_nothrow_swappable_v<T>);
        std::swap(this->native(), native);
    }

    /// Get type as type index of the ::UA_TYPES array.
    static constexpr TypeIndex getTypeIndex() {
        return typeIndex;
    }

protected:
    constexpr void clear() noexcept {
        detail::clear(this->native(), UA_TYPES[typeIndex]);
    }
};

/* -------------------------------------------- Trait ------------------------------------------- */

namespace detail {

template <typename T>
struct IsTypeWrapper {
    // https://stackoverflow.com/a/51910887
    template <typename U, TypeIndex typeIndex>
    static std::true_type check(const TypeWrapper<U, typeIndex>&);

    static std::false_type check(...);

    using type = decltype(check(std::declval<T&>()));  // NOLINT
    static constexpr bool value = type::value;
};

template <typename T>
inline constexpr bool isTypeWrapper = IsTypeWrapper<T>::value;

}  // namespace detail

}  // namespace opcua
