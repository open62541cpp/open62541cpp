cmake_minimum_required(VERSION 3.12)

project(
    open62541pp
    VERSION 0.17.0
    DESCRIPTION "C++ wrapper of open62541"
    HOMEPAGE_URL "https://github.com/open62541pp/open62541pp"
    LANGUAGES CXX
)

# set c++ standard explicitly, compile feature "cxx_std_17" does not set -std=c++17 compile flag
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# compiled binaries folders (same as open62541)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(UAPP_ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)
option(UAPP_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# interface "library" to set the c++ standard / compile-time options requested
add_library(open62541pp_project_options INTERFACE)
target_compile_features(open62541pp_project_options INTERFACE cxx_std_17)

include(cmake/Sanitizers.cmake)
include(cmake/StaticAnalyzers.cmake)

if(MSVC)
    set(warnings
        /permissive-
        /W4
        /w14640
        /wd4127  # conditional expression is constant
        /wd4661  # explicit template instantiations in cpp files
        /wd4702  # unreachable code, false positive
        /wd4996  # deprecation gmtime, localtime
    )
    if(UAPP_WARNINGS_AS_ERRORS)
        list(APPEND warnings /WX)
    endif()
    target_compile_options(open62541pp_project_options INTERFACE ${warnings})
else()
    set(warnings
        -Wall
        -Wextra
        -Wshadow
        -Wnon-virtual-dtor
        -Wpedantic
    )
    if(UAPP_WARNINGS_AS_ERRORS)
        list(APPEND warnings -Werror)
    endif()
    target_compile_options(open62541pp_project_options INTERFACE ${warnings})

    if(UAPP_ENABLE_COVERAGE)
        target_compile_options(open62541pp_project_options INTERFACE --coverage -O0 -g)
        target_link_libraries(open62541pp_project_options INTERFACE --coverage)
    endif()
endif()

# threads
find_package(Threads REQUIRED)

# open62541
option(UAPP_INTERNAL_OPEN62541 "Use internal open62541 library" ON)
if(UAPP_INTERNAL_OPEN62541)
    # disable IPO if not defined, otherwise open62541 will enable it
    # IPO increases link times, especially with sanitizers enabled
    if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
    endif()

    # overwrite default open62541 options
    # disable sanitizers in debug build (> v1.3)
    if (NOT UA_ENABLE_DEBUG_SANITIZER)
        set(UA_ENABLE_DEBUG_SANITIZER OFF CACHE BOOL "")
        mark_as_advanced(UA_ENABLE_DEBUG_SANITIZER)
    endif()
    # disable sanitizers in debug build (<= v1.3) WORKAROUND
    # https://github.com/open62541/open62541/blob/v1.3.5/CMakeLists.txt#L753-L764
    if (NOT UA_ENABLE_UNIT_TESTS_MEMCHECK)
        set(UA_ENABLE_UNIT_TESTS_MEMCHECK ON CACHE BOOL "")
        mark_as_advanced(UA_ENABLE_UNIT_TESTS_MEMCHECK)
    endif()

    # disable warnings as errors for open62541
    if(NOT UA_FORCE_WERROR)
        set(UA_FORCE_WERROR OFF OFF CACHE BOOL "")
    endif()

    add_subdirectory(3rdparty/open62541)  # target open62541::open62541
else()
    find_package(open62541 REQUIRED)
endif()

# enable IPO if open62541 is compiled with IPO
get_target_property(open62541_ipo open62541::open62541 INTERPROCEDURAL_OPTIMIZATION)
if(open62541_ipo)
    message(STATUS "IPO enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
else()
    message(STATUS "IPO not enabled")
endif()

# open62541pp library (static/shared depending on option BUILD_SHARED_LIBS)
include(GNUInstallDirs)
configure_file(
    include/open62541pp/config.hpp.in
    ${PROJECT_BINARY_DIR}/include_generated/open62541pp/config.hpp
)

function(deprecated_header header_old header_new)
    configure_file(
        ${PROJECT_SOURCE_DIR}/include/open62541pp/deprecated.hpp.in
        ${PROJECT_BINARY_DIR}/include_generated/open62541pp/${header_old}
    )
endfunction()
deprecated_header(plugins/PluginAdapter.h plugin/pluginadapter.hpp)
deprecated_header(services/Attribute_highlevel.h services/attribute_highlevel.hpp)
deprecated_header(services/Attribute.h services/attribute.hpp)
deprecated_header(services/Method.h services/method.hpp)
deprecated_header(services/MonitoredItem.h services/monitoreditem.hpp)
deprecated_header(services/NodeManagement.h services/nodemanagement.hpp)
deprecated_header(services/Subscription.h services/subscription.hpp)
deprecated_header(services/View.h services/view.hpp)
deprecated_header(services/services.h services/services.hpp)
deprecated_header(types/Builtin.h types.hpp)
deprecated_header(types/Composed.h types_composed.hpp)
deprecated_header(types/DataValue.h types.hpp)
deprecated_header(types/DateTime.h types.hpp)
deprecated_header(types/ExtensionObject.h types.hpp)
deprecated_header(types/NodeId.h types.hpp)
deprecated_header(types/Variant.h types.hpp)
deprecated_header(AccessControl.h plugin/accesscontrol_default.hpp)
deprecated_header(async.h async.hpp)
deprecated_header(Bitmask.h bitmask.hpp)
deprecated_header(Client.h client.hpp)
deprecated_header(Common.h common.hpp)
deprecated_header(Config.h config.hpp)
deprecated_header(Crypto.h plugin/create_certificate.hpp)
deprecated_header(DataType.h datatype.hpp)
deprecated_header(DataTypeBuilder.h datatype.hpp)
deprecated_header(ErrorHandling.h exception.hpp)
deprecated_header(Event.h event.hpp)
deprecated_header(Logger.h plugin/log_default.hpp)
deprecated_header(MonitoredItem.h monitoreditem.hpp)
deprecated_header(Node.h node.hpp)
deprecated_header(NodeIds.h nodeids.hpp)
deprecated_header(open62541pp.h open62541pp.hpp)
deprecated_header(Result.h result.hpp)
deprecated_header(Server.h server.hpp)
deprecated_header(Session.h session.hpp)
deprecated_header(Span.h span.hpp)
deprecated_header(Subscription.h subscription.hpp)
deprecated_header(TypeConverter.h typeconverter.hpp)
deprecated_header(TypeRegistry.h typeregistry.hpp)
deprecated_header(TypeRegistryNative.h typeregistry_generated.hpp)
deprecated_header(TypeWrapper.h typewrapper.hpp)
deprecated_header(ValueBackend.h plugin/nodestore.hpp)
deprecated_header(Wrapper.h wrapper.hpp)
deprecated_header(nodeids.hpp ua/nodeids.hpp)
deprecated_header(types_composed.hpp ua/types.hpp)

add_library(
    open62541pp
    src/client.cpp
    src/datatype.cpp
    src/event.cpp
    src/monitoreditem.cpp
    src/node.cpp
    src/plugin/accesscontrol.cpp
    src/plugin/accesscontrol_default.cpp
    src/plugin/create_certificate.cpp
    src/plugin/log.cpp
    src/plugin/nodesetloader.cpp
    src/plugin/nodestore.cpp
    src/server.cpp
    src/services_attribute.cpp
    src/services_method.cpp
    src/services_monitoreditem.cpp
    src/services_nodemanagement.cpp
    src/services_subscription.cpp
    src/services_view.cpp
    src/session.cpp
    src/string_utils.cpp
    src/subscription.cpp
    src/types.cpp
    src/ua_types.cpp
)
add_library(open62541pp::open62541pp ALIAS open62541pp)
target_include_directories(
    open62541pp
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include_generated>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(
    open62541pp
    PUBLIC
        open62541::open62541
    PRIVATE
        Threads::Threads
        $<BUILD_INTERFACE:open62541pp_project_options>
)

# open62541-nodeset-loader
option(UAPP_ENABLE_NODESETLOADER "Enable nodeset loader" OFF)
if(UAPP_ENABLE_NODESETLOADER)
    # variable OPEN62541_VERSION required by open62541-nodeset-loader
    get_target_property(OPEN62541_VERSION open62541::open62541 VERSION)
    set(NODESETLOADER_ENABLE_BUILD_INTO_OPEN62541 ON CACHE BOOL "")
    add_subdirectory(3rdparty/open62541-nodeset-loader EXCLUDE_FROM_ALL)

    # build and link as object library
    enable_language(C)
    add_library(open62541pp_nodesetloader OBJECT ${NODESETLOADER_SOURCES})
    target_include_directories(
        open62541pp_nodesetloader
        PUBLIC
            ${NODESETLOADER_PUBLIC_INCLUDES}
        PRIVATE
            ${NODESETLOADER_PRIVATE_INCLUDES}
    )
    target_link_libraries(
        open62541pp_nodesetloader
        PUBLIC
            open62541::open62541
        PRIVATE
            ${NODESETLOADER_DEPS_LIBS}
    )
    target_compile_definitions(
        open62541pp_nodesetloader
        PUBLIC
            NODESETLOADER_CLEANUP_CUSTOM_DATATYPES=1
    )
    set_target_properties(
        open62541pp_nodesetloader
        PROPERTIES
            C_VISIBILITY_PRESET hidden
            POSITION_INDEPENDENT_CODE ON  # required for shared library
    )
    target_link_libraries(open62541pp PRIVATE $<BUILD_INTERFACE:open62541pp_nodesetloader>
    )
endif()

# tests
option(UAPP_BUILD_TESTS "Build unit tests" OFF)
if(UAPP_BUILD_TESTS)
    message(STATUS "Unit tests enabled")
    enable_testing()
    add_subdirectory(tests)
endif()

# examples
option(UAPP_BUILD_EXAMPLES "Build examples" OFF)
if(UAPP_BUILD_EXAMPLES)
    message(STATUS "Examples enabled")
    add_subdirectory(examples)
endif()

# documentation
option(UAPP_BUILD_DOCUMENTATION "Build documentation" OFF)
if(UAPP_BUILD_DOCUMENTATION)
    message(STATUS "Documentation enabled")
    add_subdirectory(doc)
endif()

# install targets
install(
    TARGETS open62541pp
    EXPORT open62541ppTargets
)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

install(
    DIRECTORY "${PROJECT_BINARY_DIR}/include_generated/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp" 
)

# install cmake config files
install(
    EXPORT open62541ppTargets
    NAMESPACE open62541pp::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/open62541pp
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/open62541ppConfig.cmake.in"
    "${CMAKE_BINARY_DIR}/open62541ppConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/open62541pp
)

write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/open62541ppConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

install(
    FILES
        "${CMAKE_BINARY_DIR}/open62541ppConfigVersion.cmake"
        "${CMAKE_BINARY_DIR}/open62541ppConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/open62541pp
)
