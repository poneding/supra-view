#include <cstdlib>
#include <iostream>

#ifndef SUPRA_WORKSPACE_RENDERER_VALIDATION_LINKED
#define SUPRA_WORKSPACE_RENDERER_VALIDATION_LINKED 0
#endif

#if __has_include("core/renderer/workspace_renderer_validation.h")
#include "core/renderer/workspace_renderer_validation.h"
#define SUPRA_HAS_WORKSPACE_RENDERER_VALIDATION_HEADER 1
#else
#define SUPRA_HAS_WORKSPACE_RENDERER_VALIDATION_HEADER 0
#endif

namespace {

void Expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(1);
  }
}

}  // namespace

int main() {
  Expect(SUPRA_HAS_WORKSPACE_RENDERER_VALIDATION_HEADER == 1,
         "Expected workspace renderer validation header.");
  Expect(SUPRA_WORKSPACE_RENDERER_VALIDATION_LINKED == 1,
         "Expected workspace renderer validation implementation.");

#if SUPRA_HAS_WORKSPACE_RENDERER_VALIDATION_HEADER && SUPRA_WORKSPACE_RENDERER_VALIDATION_LINKED
  using supra::renderer::ValidateWorkspaceResolution;
  using supra::session::WorkspaceResolution;

  const WorkspaceResolution validResolution{{2560, 1440}, {1280, 720}};
  const WorkspaceResolution zeroLogicalWidth{{0, 1440}, {1280, 720}};
  const WorkspaceResolution zeroPresentationHeight{{2560, 1440}, {1280, 0}};
  const WorkspaceResolution maxD3D11Resolution{{16384, 16384}, {16384, 16384}};
  const WorkspaceResolution oversizedLogicalWidth{{16385, 1440}, {1280, 720}};
  const WorkspaceResolution oversizedPresentationHeight{{2560, 1440}, {1280, 16385}};

  Expect(ValidateWorkspaceResolution(validResolution),
         "Expected ordinary workspace resolution to validate.");
  Expect(!ValidateWorkspaceResolution(zeroLogicalWidth),
         "Expected zero logical width to fail validation.");
  Expect(!ValidateWorkspaceResolution(zeroPresentationHeight),
         "Expected zero presentation height to fail validation.");
  Expect(ValidateWorkspaceResolution(maxD3D11Resolution),
         "Expected D3D11 maximum 2D resolution to remain valid.");
  Expect(!ValidateWorkspaceResolution(oversizedLogicalWidth),
         "Expected oversized logical width to fail validation.");
  Expect(!ValidateWorkspaceResolution(oversizedPresentationHeight),
         "Expected oversized presentation height to fail validation.");
#endif

  return 0;
}
