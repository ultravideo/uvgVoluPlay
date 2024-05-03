cmake_minimum_required(VERSION 3.27)
set(PROJECT_NAME uvgVisualizer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(GLIBCXX_USE_CXX11_ABI "Set -D_GLIBCXX_USE_CXX11_ABI=1" OFF)
option(STATIC_WINDOWS_RUNTIME "Use static (MT/MTd) Windows runtime" OFF)

if(STATIC_WINDOWS_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>DLL")
endif()
if(UNIX)
    set_property(SOURCE ${SOURCES} APPEND_STRING PROPERTY COMPILE_FLAGS " -march=native")
endif()

set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")  # Use -O3 for higher optimization

# Set build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
#   set(CMAKE_BUILD_TYPE RelWithDebInfo)
  set(CMAKE_BUILD_TYPE Release)
endif()

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "imgui.ini"
    "shaders/fr_nolight.shader"
    "shaders/fs.shader"
    "shaders/fs_pbr.shader"
    "shaders/vs.shader"
)
source_group("" FILES ${no_group_source_files})

set(Headers
    "source/3rdparty/plugins/imgui/ImFileBrowser.h"
    "source/application.h"
    "source/elems/pointcloud.h"
    "source/elems/camera.h"
    "source/elems/element.h"
    "source/elems/face.h"
    "source/elems/input.h"
    "source/elems/light.h"
    "source/elems/mesh.h"
    "source/elems/vertex_holder.h"
    "source/main.h"
    "source/pch.h"
    "source/ply_import/miniply.h"
    "source/render/opengl_buffer_manager.h"
    "source/render/opengl_context.h"
    "source/render/render_base.h"
    "source/render/ui_context.h"
    "source/shader/shader_util.h"
    "source/ui/pcl_property_panel.h"
    "source/ui/property_panel.h"
    "source/ui/scene_view.h"
    "source/ui/stat_panel.h"
    "source/utils/imgui_widgets.h"
    "source/utils/str_utils.h"
    "source/window/gl_window.h"
    "source/window/window.h"
)
source_group("Headers" FILES ${Headers})

set(Sources
    "source/3rdparty/imgui-docking/backends/imgui_impl_glfw.cpp"
    "source/3rdparty/imgui-docking/backends/imgui_impl_opengl3.cpp"
    "source/3rdparty/imgui-docking/imgui.cpp"
    "source/3rdparty/imgui-docking/imgui_demo.cpp"
    "source/3rdparty/imgui-docking/imgui_draw.cpp"
    "source/3rdparty/imgui-docking/imgui_tables.cpp"
    "source/3rdparty/imgui-docking/imgui_widgets.cpp"
    "source/application.cpp"
    "source/elems/pointcloud.cpp"
    "source/elems/element.cpp"
    "source/elems/face.cpp"
    "source/elems/mesh.cpp"
    "source/elems/vertex_holder.cpp"
    "source/main.cpp"
    "source/pch.cpp"
    "source/ply_import/miniply.cpp"
    "source/render/opengl_buffer_manager.cpp"
    "source/render/opengl_context.cpp"
    "source/render/ui_context.cpp"
    "source/shader/shader_util.cpp"
    "source/ui/pcl_property_panel.cpp"
    "source/ui/property_panel.cpp"
    "source/ui/scene_view.cpp"
    "source/ui/stat_panel.cpp"
    "source/window/gl_window.cpp"
)
source_group("Sources" FILES ${Sources})

set(ALL_FILES
    ${no_group_source_files}
    ${Headers}
    ${Sources}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/source/pch.h>"
)
set(ROOT_NAMESPACE uvg_Visualizer)

################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/source;"
        "${CMAKE_CURRENT_SOURCE_DIR}/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/3rdparty/imgui-docking;"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/3rdparty/imgui-docking/backends;"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/3rdparty/plugins/imgui"
    )
    # Include nlohmann_json headers
    # target_include_directories(${PROJECT_NAME} PRIVATE ${NLOHMANN_JSON_INSTALL_DIR}/include)
    list(APPEND CMAKE_PREFIX_PATH "./vcpkg_installed/x64-windows/share/cppzmq")
    list(APPEND CMAKE_PREFIX_PATH "./vcpkg_installed/x64-windows/share/zeromq")

    find_package(cppzmq CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE cppzmq cppzmq-static)

    if (OPENMP_FOUND)
        target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
    endif()

endif()

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "GLEW_STATIC;"
        "_MBCS"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "_MBCS"
    )
endif()

################################################################################
# Dependencies
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "glfw3;"
        "glew32s;"
        "opengl32"
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
# target_link_libraries(${PROJECT_NAME} PRIVATE k4a::k4a)

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}//libs/gl;"
        "${CMAKE_CURRENT_SOURCE_DIR}//libs/glfw"
    )
endif()

# Copy folder A to build/tool/
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders
     DESTINATION ${CMAKE_BINARY_DIR}/Release)