add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/doctest doctest EXCLUDE_FROM_ALL)  # don't install

add_executable(
    open62541pp_tests
    main.cpp
    async.cpp
    bitmask.cpp
    client_server_common.cpp
    client_service.cpp
    client.cpp
    datatype.cpp
    event.cpp
    exception.cpp
    exceptioncatcher.cpp
    iterator.cpp
    node.cpp
    plugin_accesscontrol.cpp
    plugin_create_certificate.cpp
    plugin_log.cpp
    plugin_nodesetloader.cpp
    pluginadapter.cpp
    result.cpp
    scope.cpp
    server.cpp
    services_attribute.cpp
    services_helper.cpp
    services_method.cpp
    services_monitoreditem.cpp
    services_nodemanagement.cpp
    services_subscription.cpp
    services_view.cpp
    session.cpp
    span.cpp
    string_utils.cpp
    subscription_monitoreditem.cpp
    traits.cpp
    typeconverter.cpp
    typeregistry.cpp
    types_builtin.cpp
    types_handling.cpp
    ua_types.cpp
    wrapper.cpp
)
target_link_libraries(
    open62541pp_tests
    PRIVATE
        doctest::doctest
        open62541pp::open62541pp
        open62541pp_project_options
)
target_include_directories(open62541pp_tests PRIVATE ../src)
set_target_properties(
    open62541pp_tests
    PROPERTIES
        OUTPUT_NAME tests
        CXX_CLANG_TIDY ""  # disable clang-tidy
)
target_compile_definitions(
    open62541pp_tests
    PRIVATE
        UAPP_NODESET_DIR="${PROJECT_SOURCE_DIR}/3rdparty/open62541/deps/ua-nodeset"
)
# fix LNK4096 error with MSVC
# https://learn.microsoft.com/en-us/cpp/error-messages/tool-errors/linker-tools-warning-lnk4098
if(MSVC)
    set_target_properties(
        open62541pp_tests
        PROPERTIES
            LINK_FLAGS "/NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib"
    )
endif()

# discover tests for ctest
include(${PROJECT_SOURCE_DIR}/3rdparty/doctest/scripts/cmake/doctest.cmake)
doctest_discover_tests(open62541pp_tests)
