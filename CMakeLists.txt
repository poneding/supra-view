cmake_minimum_required(VERSION 3.20)

project(supra_view LANGUAGES CXX)

if(NOT WIN32)
  message(FATAL_ERROR "supra_view currently builds only on Windows because it requires Win32, DXGI Desktop Duplication, and Direct3D 11.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(supra_view WIN32
  app/main.cpp
  app/app_controller.cpp
  app/window.cpp
  core/capture/capture_utils.cpp
  core/capture/desktop_capture.cpp
  core/renderer/d3d_context.cpp
  core/renderer/texture_pipeline.cpp
  core/renderer/window_renderer.cpp
  core/shader/fullscreen_quad.cpp
  core/shader/shader_compiler.cpp
  core/shader/shader_sources.cpp
)

target_include_directories(supra_view PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(supra_view PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)

if(MSVC)
  target_compile_options(supra_view PRIVATE /W4 /permissive- /EHsc)
endif()

target_link_libraries(supra_view PRIVATE
  d3d11
  dxgi
  d3dcompiler
  user32
  gdi32
  ole32
)
