#pragma once

#include <source_location>
#include <string_view>

namespace anvil::core {

[[noreturn]] void assertion_failed(
    std::string_view expression,
    std::string_view message,
    const std::source_location& location = std::source_location::current());

} // namespace anvil::core

#define ANVIL_ASSERT(expression, message)                                                          \
    do {                                                                                           \
        if (!(expression)) [[unlikely]] {                                                          \
            ::anvil::core::assertion_failed(#expression, (message));                               \
        }                                                                                          \
    } while (false)
