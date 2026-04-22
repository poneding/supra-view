cmake_minimum_required(VERSION 3.20)

project(supra_view LANGUAGES CXX)

option(SUPRA_VIEW_BUILD_WORKSPACE_PLACEHOLDER_SCENE_TEST
  "Build the standalone workspace placeholder scene regression test."
  OFF)

if(NOT WIN32 AND NOT SUPRA_VIEW_BUILD_WORKSPACE_PLACEHOLDER_SCENE_TEST)
  message(FATAL_ERROR "supra_view currently builds only on Windows because it requires Win32, DXGI Desktop Duplication, and Direct3D 11.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
  add_executable(supra_view WIN32
    app/main.cpp
    app/v2_app_controller.cpp
    core/input/input_mapper.cpp
    core/input/input_state.cpp
    core/session/workspace_session.cpp
    core/session/workspace_state.cpp
    app/window.cpp
    core/renderer/d3d_context.cpp
    core/renderer/texture_pipeline.cpp
    core/renderer/workspace_downsample_pass.cpp
    core/renderer/workspace_placeholder_scene.cpp
    core/renderer/workspace_renderer.cpp
    core/renderer/workspace_renderer_validation.cpp
    core/renderer/window_renderer.cpp
    core/shader/fullscreen_quad.cpp
    core/shader/shader_compiler.cpp
    core/shader/shader_sources.cpp
    core/shader/workspace_downsample_shader_contract.cpp
    core/shader/workspace_shader_sources.cpp
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
endif()

if(SUPRA_VIEW_BUILD_WORKSPACE_PLACEHOLDER_SCENE_TEST)
  enable_testing()

  add_executable(workspace_placeholder_scene_test
    core/renderer/workspace_placeholder_scene.cpp
    core/renderer/workspace_placeholder_scene_test.cpp
  )

  target_include_directories(workspace_placeholder_scene_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  if(MSVC)
    target_compile_options(workspace_placeholder_scene_test PRIVATE /W4 /permissive- /EHsc)
  endif()

  add_test(NAME workspace_placeholder_scene_test COMMAND workspace_placeholder_scene_test)

  add_executable(workspace_renderer_validation_test
    core/renderer/workspace_renderer_validation_test.cpp
  )

  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/renderer/workspace_renderer_validation.cpp")
    target_sources(workspace_renderer_validation_test PRIVATE
      core/renderer/workspace_renderer_validation.cpp
    )
    target_compile_definitions(workspace_renderer_validation_test PRIVATE SUPRA_WORKSPACE_RENDERER_VALIDATION_LINKED=1)
  else()
    target_compile_definitions(workspace_renderer_validation_test PRIVATE SUPRA_WORKSPACE_RENDERER_VALIDATION_LINKED=0)
  endif()

  target_include_directories(workspace_renderer_validation_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  if(MSVC)
    target_compile_options(workspace_renderer_validation_test PRIVATE /W4 /permissive- /EHsc)
  endif()

  add_test(NAME workspace_renderer_validation_test COMMAND workspace_renderer_validation_test)

  add_executable(workspace_downsample_pass_test
    core/renderer/workspace_downsample_pass_test.cpp
  )

  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/renderer/workspace_downsample_pass.cpp")
    target_sources(workspace_downsample_pass_test PRIVATE
      core/renderer/workspace_downsample_pass.cpp
    )
    if(WIN32)
      target_sources(workspace_downsample_pass_test PRIVATE
        core/shader/fullscreen_quad.cpp
        core/shader/shader_compiler.cpp
        core/shader/workspace_downsample_shader_contract.cpp
        core/shader/workspace_shader_sources.cpp
      )
      target_link_libraries(workspace_downsample_pass_test PRIVATE
        d3d11
        d3dcompiler
      )
    endif()
    target_compile_definitions(workspace_downsample_pass_test PRIVATE SUPRA_WORKSPACE_DOWNSAMPLE_PASS_LINKED=1)
  else()
    target_compile_definitions(workspace_downsample_pass_test PRIVATE SUPRA_WORKSPACE_DOWNSAMPLE_PASS_LINKED=0)
  endif()

  target_include_directories(workspace_downsample_pass_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  if(MSVC)
    target_compile_options(workspace_downsample_pass_test PRIVATE /W4 /permissive- /EHsc)
  endif()

  add_test(NAME workspace_downsample_pass_test COMMAND workspace_downsample_pass_test)

  add_executable(workspace_shader_sources_test
    core/shader/workspace_shader_sources_test.cpp
  )

  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/shader/workspace_shader_sources.cpp")
    target_sources(workspace_shader_sources_test PRIVATE
      core/shader/workspace_shader_sources.cpp
    )
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/shader/workspace_downsample_shader_contract.cpp")
      target_sources(workspace_shader_sources_test PRIVATE
        core/shader/workspace_downsample_shader_contract.cpp
      )
    endif()
    target_compile_definitions(workspace_shader_sources_test PRIVATE SUPRA_WORKSPACE_SHADER_SOURCES_LINKED=1)
  else()
    target_compile_definitions(workspace_shader_sources_test PRIVATE SUPRA_WORKSPACE_SHADER_SOURCES_LINKED=0)
  endif()

  target_include_directories(workspace_shader_sources_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  if(MSVC)
    target_compile_options(workspace_shader_sources_test PRIVATE /W4 /permissive- /EHsc)
  endif()

  add_test(NAME workspace_shader_sources_test COMMAND workspace_shader_sources_test)

  add_executable(workspace_downsample_shader_contract_test
    core/shader/workspace_downsample_shader_contract_test.cpp
  )

  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/shader/workspace_downsample_shader_contract.cpp")
    target_sources(workspace_downsample_shader_contract_test PRIVATE
      core/shader/workspace_downsample_shader_contract.cpp
    )
    target_compile_definitions(workspace_downsample_shader_contract_test PRIVATE SUPRA_WORKSPACE_DOWNSAMPLE_SHADER_CONTRACT_LINKED=1)
  else()
    target_compile_definitions(workspace_downsample_shader_contract_test PRIVATE SUPRA_WORKSPACE_DOWNSAMPLE_SHADER_CONTRACT_LINKED=0)
  endif()

  target_include_directories(workspace_downsample_shader_contract_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  if(MSVC)
    target_compile_options(workspace_downsample_shader_contract_test PRIVATE /W4 /permissive- /EHsc)
  endif()

  add_test(NAME workspace_downsample_shader_contract_test COMMAND workspace_downsample_shader_contract_test)
endif()
