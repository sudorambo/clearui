cmake_minimum_required(VERSION 3.15)
project(ClearUI VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------

option(CLEARUI_EMBED_FONT "Embed the default TTF font as a C byte array" OFF)
option(CLEARUI_BUILD_EXAMPLES "Build example programs" OFF)

# ---------------------------------------------------------------------------
# Source lists
# ---------------------------------------------------------------------------

set(CORE_SRCS
    src/core/arena.c
    src/core/frame_alloc.c
    src/core/vault.c
    src/core/utf8.c
    src/core/version.c
    src/core/context.c
    src/core/draw_cmd.c
    src/core/node.c
    src/core/diff.c
    src/core/render.c
    src/core/a11y.c
)

set(FONT_SRCS   src/font/atlas.c)
set(LAYOUT_SRCS src/layout/layout.c)

set(WIDGET_SRCS
    src/widget/layout.c
    src/widget/label.c
    src/widget/button.c
    src/widget/checkbox.c
    src/widget/spacer.c
    src/widget/icon_button.c
    src/widget/scroll.c
    src/widget/text_input.c
    src/widget/canvas.c
)

set(RDI_SRCS  src/rdi/clearui_rdi_soft.c)
set(PLAT_SRCS src/platform/cui_platform_stub.c)

set(LIB_SRCS ${CORE_SRCS} ${FONT_SRCS} ${LAYOUT_SRCS} ${WIDGET_SRCS})

# ---------------------------------------------------------------------------
# Embedded font generation (optional)
# ---------------------------------------------------------------------------

if(CLEARUI_EMBED_FONT)
    set(EMBED_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/src/font/default_font_embed.h")
    set(FONT_TTF     "${CMAKE_CURRENT_SOURCE_DIR}/deps/default_font.ttf")

    add_custom_command(
        OUTPUT  "${EMBED_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E echo "#ifndef CLEARUI_DEFAULT_FONT_EMBED_H"              > "${EMBED_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E echo "#define CLEARUI_DEFAULT_FONT_EMBED_H"             >> "${EMBED_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E echo ""                                                 >> "${EMBED_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E echo "static const unsigned char clearui_default_font_ttf[] = {" >> "${EMBED_HEADER}"
        COMMAND xxd -i < "${FONT_TTF}" >> "${EMBED_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E echo "}$<SEMICOLON>"                                   >> "${EMBED_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E echo "static const unsigned int clearui_default_font_ttf_len = " >> "${EMBED_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_FILE_DIR:clearui>"                      >> "${EMBED_HEADER}"
        DEPENDS "${FONT_TTF}"
        COMMENT "Generating embedded font header"
    )

    add_custom_target(clearui_embed_font DEPENDS "${EMBED_HEADER}")
endif()

# ---------------------------------------------------------------------------
# Library targets
# ---------------------------------------------------------------------------

# Core library (matches `make lib` — no RDI, no platform stub)
add_library(clearui STATIC ${LIB_SRCS})
target_include_directories(clearui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)
target_link_libraries(clearui PRIVATE m)
target_compile_options(clearui PRIVATE -Wall -Wextra -Wpedantic)

if(CLEARUI_EMBED_FONT)
    target_compile_definitions(clearui PRIVATE CLEARUI_EMBED_FONT)
    add_dependencies(clearui clearui_embed_font)
endif()

# Overlay library (matches `make overlay` — core + soft RDI + platform stub)
add_library(clearui_overlay STATIC ${LIB_SRCS} ${RDI_SRCS} ${PLAT_SRCS})
target_include_directories(clearui_overlay
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)
target_link_libraries(clearui_overlay PRIVATE m)
target_compile_options(clearui_overlay PRIVATE -Wall -Wextra -Wpedantic)

if(CLEARUI_EMBED_FONT)
    target_compile_definitions(clearui_overlay PRIVATE CLEARUI_EMBED_FONT)
    add_dependencies(clearui_overlay clearui_embed_font)
endif()

# ---------------------------------------------------------------------------
# Examples (opt-in)
# ---------------------------------------------------------------------------

if(CLEARUI_BUILD_EXAMPLES)
    add_executable(demo examples/demo.c)
    target_link_libraries(demo PRIVATE clearui_overlay)
    target_include_directories(demo PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    )
endif()

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS clearui clearui_overlay
    EXPORT ClearUITargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY shaders/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/clearui/shaders
    FILES_MATCHING
        PATTERN "*.vert"
        PATTERN "*.frag"
        PATTERN "*.spv"
)

# CMake package config so downstream can find_package(ClearUI)
install(EXPORT ClearUITargets
    FILE ClearUITargets.cmake
    NAMESPACE ClearUI::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ClearUI
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ClearUIConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ClearUIConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ClearUI
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ClearUIConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ClearUIConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ClearUIConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ClearUI
)

# pkg-config
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/clearui.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/clearui.pc"
    @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/clearui.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
