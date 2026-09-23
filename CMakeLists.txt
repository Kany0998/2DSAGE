cmake_minimum_required(VERSION 3.25)

project(2DSAGE VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(lua CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

set(IMGUI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_sdl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_impl_sdl.cpp"
)

add_executable(2DSAGE
    ${PROJECT_SOURCES}
    ${IMGUI_SOURCES}
 "src/Components/ManaComponent.h" "src/Systems/RenderManaBarSystem.h" "src/Components/AttributesComponent.h" "src/Systems/ManaRegenerationSystem.h" "src/Systems/HealthRegenerationSystem.h")

target_include_directories(2DSAGE PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs"
)

target_link_libraries(2DSAGE PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
    SDL2_image::SDL2_image
    sdl_ttf::sdl_ttf
    SDL2_mixer::SDL2_mixer
    glm::glm-header-only
    sol2::sol2
    lua::lua
    EnTT::EnTT
)

if(MSVC)
    target_compile_options(2DSAGE PRIVATE
        /W4
        /permissive-
        /Zc:__cplusplus
    )
else()
    target_compile_options(2DSAGE PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*"
)

# A POST_BUILD step on 2DSAGE only runs when 2DSAGE itself needs to relink -
# editing just a .lua/asset file with no C++ changes would make the build
# system decide there's nothing to do and skip the copy entirely, leaving a
# stale assets/ folder next to the exe. Depending on the actual asset files
# (via this stamp) means the copy re-runs whenever any asset changes, whether
# or not any C++ source changed.
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/assets_copied.stamp"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:2DSAGE>/assets"
    COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_BINARY_DIR}/assets_copied.stamp"
    DEPENDS ${ASSET_FILES}
    COMMENT "Copying assets to output directory"
)

add_custom_target(CopyAssets ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/assets_copied.stamp"
)

add_dependencies(2DSAGE CopyAssets)

if(WIN32)
    add_custom_command(
        TARGET 2DSAGE POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:2DSAGE>
            $<TARGET_FILE_DIR:2DSAGE>
        COMMAND_EXPAND_LISTS
    )
endif()