#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

#if __has_include("core/shader/workspace_downsample_shader_contract.h")
#include "core/shader/workspace_downsample_shader_contract.h"
#define SUPRA_HAS_WORKSPACE_DOWNSAMPLE_SHADER_CONTRACT_HEADER 1
#else
#define SUPRA_HAS_WORKSPACE_DOWNSAMPLE_SHADER_CONTRACT_HEADER 0
#endif

#ifndef SUPRA_WORKSPACE_DOWNSAMPLE_SHADER_CONTRACT_LINKED
#define SUPRA_WORKSPACE_DOWNSAMPLE_SHADER_CONTRACT_LINKED 0
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

bool NearlyEqual(float lhs, float rhs) {
  return std::fabs(lhs - rhs) < 0.00001f;
}

}  // namespace

int main() {
  Expect(SUPRA_HAS_WORKSPACE_DOWNSAMPLE_SHADER_CONTRACT_HEADER == 1,
         "Expected workspace downsample shader contract header.");
  Expect(SUPRA_WORKSPACE_DOWNSAMPLE_SHADER_CONTRACT_LINKED == 1,
         "Expected workspace downsample shader contract implementation.");

#if SUPRA_HAS_WORKSPACE_DOWNSAMPLE_SHADER_CONTRACT_HEADER && \
    SUPRA_WORKSPACE_DOWNSAMPLE_SHADER_CONTRACT_LINKED
  const auto constants = supra::shader::BuildWorkspaceDownsampleConstants(2560U, 1440U, 1280U, 720U);
  const char* hlsl = supra::shader::WorkspaceDownsampleConstantBufferHlslSource();

  Expect(sizeof(constants) == 32U,
         "Expected workspace downsample constants to stay tightly packed for a single constant buffer.");
  Expect(NearlyEqual(constants.logicalSourceSize[0], 2560.0f),
         "Expected contract helper to keep logical source width.");
  Expect(NearlyEqual(constants.logicalSourceSize[1], 1440.0f),
         "Expected contract helper to keep logical source height.");
  Expect(NearlyEqual(constants.inverseLogicalSourceSize[0], 1.0f / 2560.0f),
         "Expected contract helper to precompute inverse logical width.");
  Expect(NearlyEqual(constants.inverseLogicalSourceSize[1], 1.0f / 1440.0f),
         "Expected contract helper to precompute inverse logical height.");
  Expect(NearlyEqual(constants.presentationTargetSize[0], 1280.0f),
         "Expected contract helper to keep presentation target width.");
  Expect(NearlyEqual(constants.presentationTargetSize[1], 720.0f),
         "Expected contract helper to keep presentation target height.");
  Expect(NearlyEqual(constants.inversePresentationTargetSize[0], 1.0f / 1280.0f),
         "Expected contract helper to precompute inverse presentation width.");
  Expect(NearlyEqual(constants.inversePresentationTargetSize[1], 1.0f / 720.0f),
         "Expected contract helper to precompute inverse presentation height.");
  Expect(Contains(hlsl, "cbuffer WorkspaceDownsampleConstants : register(b0)"),
         "Expected contract helper to declare the V2 workspace downsample constant buffer.");
  Expect(Contains(hlsl, "float2 gLogicalSourceSize;"),
         "Expected contract helper to expose logical source size.");
  Expect(Contains(hlsl, "float2 gInvLogicalSourceSize;"),
         "Expected contract helper to expose inverse logical source size.");
  Expect(Contains(hlsl, "float2 gPresentationTargetSize;"),
         "Expected contract helper to expose presentation target size.");
  Expect(Contains(hlsl, "float2 gInvPresentationTargetSize;"),
         "Expected contract helper to expose inverse presentation target size.");
#endif

  return 0;
}
