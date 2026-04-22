#include "workspace_downsample_shader_contract.h"

namespace supra {
namespace shader {

namespace {

float InverseOrZero(unsigned int extent) noexcept {
  return extent == 0U ? 0.0f : 1.0f / static_cast<float>(extent);
}

}  // namespace

WorkspaceDownsampleConstants BuildWorkspaceDownsampleConstants(
    unsigned int logicalWidth, unsigned int logicalHeight, unsigned int presentationWidth,
    unsigned int presentationHeight) noexcept {
  WorkspaceDownsampleConstants constants{};
  constants.logicalSourceSize[0] = static_cast<float>(logicalWidth);
  constants.logicalSourceSize[1] = static_cast<float>(logicalHeight);
  constants.inverseLogicalSourceSize[0] = InverseOrZero(logicalWidth);
  constants.inverseLogicalSourceSize[1] = InverseOrZero(logicalHeight);
  constants.presentationTargetSize[0] = static_cast<float>(presentationWidth);
  constants.presentationTargetSize[1] = static_cast<float>(presentationHeight);
  constants.inversePresentationTargetSize[0] = InverseOrZero(presentationWidth);
  constants.inversePresentationTargetSize[1] = InverseOrZero(presentationHeight);
  return constants;
}

const char* WorkspaceDownsampleConstantBufferHlslSource() noexcept {
  return R"(
cbuffer WorkspaceDownsampleConstants : register(b0) {
  float2 gLogicalSourceSize;
  float2 gInvLogicalSourceSize;
  float2 gPresentationTargetSize;
  float2 gInvPresentationTargetSize;
};
  )";
}

}  // namespace shader
}  // namespace supra
