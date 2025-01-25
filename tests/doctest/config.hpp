#pragma once

/**
 * Doctest configuration.
 * This include file is automatically included by CMake before any other includes.
 */

// avoid calling opcua::toString through ADL
#define DOCTEST_STRINGIFY(...) ::doctest::toString(__VA_ARGS__)
