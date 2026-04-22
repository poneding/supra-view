#pragma once

namespace supra {
namespace shader {

struct alignas(16) WorkspaceDownsampleConstants {
  float logicalSourceSize[2] = {0.0f, 0.0f};
  float inverseLogicalSourceSize[2] = {0.0f, 0.0f};
  float presentationTargetSize[2] = {0.0f, 0.0f};
  float inversePresentationTargetSize[2] = {0.0f, 0.0f};
};

static_assert(sizeof(WorkspaceDownsampleConstants) == 32U,
              "Workspace downsample constants must stay tightly packed.");
static_assert(sizeof(WorkspaceDownsampleConstants) % 16U == 0U,
              "Workspace downsample constant buffer must remain 16-byte aligned.");

[[nodiscard]] WorkspaceDownsampleConstants BuildWorkspaceDownsampleConstants(
    unsigned int logicalWidth, unsigned int logicalHeight, unsigned int presentationWidth,
    unsigned int presentationHeight) noexcept;

[[nodiscard]] const char* WorkspaceDownsampleConstantBufferHlslSource() noexcept;

}  // namespace shader
}  // namespace supra
