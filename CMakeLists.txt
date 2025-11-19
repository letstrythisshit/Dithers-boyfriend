cmake_minimum_required(VERSION 3.15)
project(DithersBoyfriend VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_GUI "Build GUI version" ON)
option(BUILD_CLI "Build CLI version" ON)

# Find packages
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs videoio highgui)

# ImGui setup
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")
if(NOT EXISTS "${IMGUI_DIR}")
    message(STATUS "Downloading Dear ImGui...")
    execute_process(
        COMMAND git clone --depth 1 https://github.com/ocornut/imgui.git "${IMGUI_DIR}"
        RESULT_VARIABLE GIT_RESULT
    )
    if(NOT GIT_RESULT EQUAL "0")
        message(FATAL_ERROR "Failed to download Dear ImGui")
    endif()
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific
    add_definitions(-DWIN32 -D_WINDOWS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

    # GLFW - use vcpkg or manual installation
    find_package(glfw3 CONFIG REQUIRED)
    set(OPENGL_LIBRARIES opengl32)
elseif(APPLE)
    # macOS-specific
    find_package(glfw3 REQUIRED)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    set(PLATFORM_LIBS ${OPENGL_LIBRARY} ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
else()
    # Linux-specific
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)
    set(PLATFORM_LIBS GL)
endif()

# Dithering library
add_library(dithering STATIC
    src/dithering.cpp
    src/dithering.h
)
target_link_libraries(dithering PUBLIC ${OpenCV_LIBS})
target_include_directories(dithering PUBLIC ${OpenCV_INCLUDE_DIRS})

# GUI version
if(BUILD_GUI)
    # ImGui sources
    set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )

    add_executable(dithers-boyfriend
        src/main.cpp
        ${IMGUI_SOURCES}
    )

    target_include_directories(dithers-boyfriend PRIVATE
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(dithers-boyfriend PRIVATE
        dithering
        glfw
        ${PLATFORM_LIBS}
        ${OpenCV_LIBS}
    )

    # Windows-specific GUI settings
    if(WIN32)
        # Use WIN32 to avoid console window in release builds
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            set_target_properties(dithers-boyfriend PROPERTIES WIN32_EXECUTABLE TRUE)
        endif()

        # Copy OpenCV DLLs on Windows
        if(OpenCV_FOUND)
            file(GLOB OpenCV_DLLS "${OpenCV_DIR}/../../bin/*.dll")
            foreach(dll ${OpenCV_DLLS})
                add_custom_command(TARGET dithers-boyfriend POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${dll} $<TARGET_FILE_DIR:dithers-boyfriend>
                )
            endforeach()
        endif()
    endif()
endif()

# CLI version
if(BUILD_CLI)
    add_executable(dithers-boyfriend-cli
        src/cli.cpp
    )

    target_include_directories(dithers-boyfriend-cli PRIVATE
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(dithers-boyfriend-cli PRIVATE
        dithering
        ${OpenCV_LIBS}
    )

    # Windows console application
    if(WIN32)
        set_target_properties(dithers-boyfriend-cli PROPERTIES
            WIN32_EXECUTABLE FALSE
        )
    endif()
endif()

# Installation
install(TARGETS dithers-boyfriend dithers-boyfriend-cli
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# CPack for packaging
set(CPACK_PACKAGE_NAME "DithersBoyfriend")
set(CPACK_PACKAGE_VENDOR "DithersBoyfriend")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced Image Dithering Application")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Dither's Boyfriend")
    set(CPACK_NSIS_PACKAGE_NAME "Dither's Boyfriend")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;ZIP")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "DithersBoyfriend")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libopencv-dev, libglfw3, libgl1")
endif()

include(CPack)

# Print build configuration
message(STATUS "===========================================")
message(STATUS "Dither's Boyfriend Build Configuration")
message(STATUS "===========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libs: ${OpenCV_LIBS}")
message(STATUS "Build GUI: ${BUILD_GUI}")
message(STATUS "Build CLI: ${BUILD_CLI}")
message(STATUS "===========================================")
