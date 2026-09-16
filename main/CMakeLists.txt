set(APP_REQUIRES
    nvs_flash esp_wifi esp_netif esp_event freertos
    esp_hid esp_hw_support esp_driver_gpio
)

# ESP-IDF first reads this file in script mode to discover dependencies.
# Both implementations use the same component dependencies.
if(CMAKE_BUILD_EARLY_EXPANSION)
    idf_component_register(INCLUDE_DIRS "../include" REQUIRES ${APP_REQUIRES})
    return()
endif()

if(IMPL STREQUAL "cpp")

    message(STATUS "Building C++26 implementation")
    file(GLOB_RECURSE CPP_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )
    idf_component_register(
        SRCS ${CPP_SOURCES}
        INCLUDE_DIRS "../include"
        REQUIRES ${APP_REQUIRES}
    )
    target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_26)

elseif(IMPL STREQUAL "zig")

    message(STATUS "Building Zig implementation")
    idf_component_register(
        SRCS "zig_headers.c"
        REQUIRES ${APP_REQUIRES}
    )

    find_program(ZIG_EXECUTABLE NAMES zig REQUIRED)
    set(ZIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Zig")
    set(ZIG_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/zig-out")
    set(ZIG_LIB "${ZIG_PREFIX}/lib/libzig_app.a")

    # Let Zig's cache check sources and imported C headers on each build.
    # Configuration and compile_commands.json exist by the time this runs.
    add_custom_target(zig_build
        COMMAND "${ZIG_EXECUTABLE}" build
            --prefix "${ZIG_PREFIX}"
            --global-cache-dir "${CMAKE_CURRENT_BINARY_DIR}/zig-global-cache"
            "-Didf-build-dir=${CMAKE_BINARY_DIR}"
        BYPRODUCTS "${ZIG_LIB}"
        WORKING_DIRECTORY "${ZIG_DIR}"
        COMMENT "Building Zig firmware library"
        VERBATIM
    )

    # main contains the C wrappers called by Zig. The declared dependencies
    # make the linker rescan static archives to resolve these calls.
    add_prebuilt_library(zig_app "${ZIG_LIB}"
        REQUIRES main ${APP_REQUIRES}
    )
    add_dependencies(zig_app zig_build)
    target_link_libraries(${COMPONENT_LIB} PRIVATE zig_app)
endif()
