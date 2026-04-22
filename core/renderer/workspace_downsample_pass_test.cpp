#include <cstdlib>
#include <iostream>

#if __has_include("core/renderer/workspace_downsample_pass.h")
#include "core/renderer/workspace_downsample_pass.h"
#define SUPRA_HAS_WORKSPACE_DOWNSAMPLE_PASS_HEADER 1
#else
#define SUPRA_HAS_WORKSPACE_DOWNSAMPLE_PASS_HEADER 0
#endif

#ifndef SUPRA_WORKSPACE_DOWNSAMPLE_PASS_LINKED
#define SUPRA_WORKSPACE_DOWNSAMPLE_PASS_LINKED 0
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
  Expect(SUPRA_HAS_WORKSPACE_DOWNSAMPLE_PASS_HEADER == 1,
         "Expected workspace downsample pass header.");
  Expect(SUPRA_WORKSPACE_DOWNSAMPLE_PASS_LINKED == 1,
         "Expected workspace downsample pass implementation.");

#if SUPRA_HAS_WORKSPACE_DOWNSAMPLE_PASS_HEADER && SUPRA_WORKSPACE_DOWNSAMPLE_PASS_LINKED
  supra::renderer::WorkspaceDownsamplePass pass;
  pass.Shutdown();
#endif

  return 0;
}
