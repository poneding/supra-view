#include <cstdlib>
#include <cstring>
#include <iostream>

#if __has_include("core/shader/workspace_shader_sources.h")
#include "core/shader/workspace_shader_sources.h"
#define SUPRA_HAS_WORKSPACE_SHADER_SOURCES_HEADER 1
#else
#define SUPRA_HAS_WORKSPACE_SHADER_SOURCES_HEADER 0
#endif

#ifndef SUPRA_WORKSPACE_SHADER_SOURCES_LINKED
#define SUPRA_WORKSPACE_SHADER_SOURCES_LINKED 0
#endif

namespace {

void Expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(1);
  }
}

bool Contains(const char* text, const char* needle) {
  return text != nullptr && needle != nullptr && std::strstr(text, needle) != nullptr;
}

}  // namespace

int main() {
  Expect(SUPRA_HAS_WORKSPACE_SHADER_SOURCES_HEADER == 1,
         "Expected explicit V2 workspace shader source header.");
  Expect(SUPRA_WORKSPACE_SHADER_SOURCES_LINKED == 1,
         "Expected explicit V2 workspace shader source implementation.");

#if SUPRA_HAS_WORKSPACE_SHADER_SOURCES_HEADER && SUPRA_WORKSPACE_SHADER_SOURCES_LINKED
  const char* fullscreenVertexSource = supra::shader::WorkspaceFullscreenVertexShaderSource();
  const char* bicubicPixelSource = supra::shader::WorkspaceBicubicDownsamplePixelShaderSource();

  Expect(Contains(fullscreenVertexSource, "WorkspaceFullscreenVS"),
         "Expected explicit workspace fullscreen vertex shader entry point.");
  Expect(Contains(fullscreenVertexSource, "SV_VertexID"),
         "Expected workspace fullscreen vertex shader to use a fullscreen triangle path.");
  Expect(Contains(bicubicPixelSource, "WorkspaceBicubicDownsamplePS"),
         "Expected explicit workspace bicubic pixel shader entry point.");
  Expect(Contains(bicubicPixelSource, "WorkspaceDownsampleConstants"),
         "Expected workspace bicubic pixel shader to declare dedicated downsample constants.");
  Expect(Contains(bicubicPixelSource, "gLogicalSourceSize"),
         "Expected workspace bicubic shader to sample from the logical target size.");
  Expect(Contains(bicubicPixelSource, "gPresentationTargetSize"),
         "Expected workspace bicubic shader to include presentation target sizing.");
#endif

  return 0;
}
