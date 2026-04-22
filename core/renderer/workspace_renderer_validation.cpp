#include "workspace_renderer_validation.h"

namespace {

constexpr unsigned int kMaxWorkspaceTextureDimension = 16384U;

bool ValidateWorkspaceExtent(unsigned int extent) noexcept {
  return extent > 0U && extent <= kMaxWorkspaceTextureDimension;
}

}  // namespace

namespace supra {
namespace renderer {

bool ValidateWorkspaceResolution(const supra::session::WorkspaceResolution& resolution) noexcept {
  return ValidateWorkspaceExtent(resolution.logical.width) &&
         ValidateWorkspaceExtent(resolution.logical.height) &&
         ValidateWorkspaceExtent(resolution.presentation.width) &&
         ValidateWorkspaceExtent(resolution.presentation.height);
}

bool ValidateWorkspacePresentationSize(unsigned int width, unsigned int height) noexcept {
  return ValidateWorkspaceExtent(width) && ValidateWorkspaceExtent(height);
}

}  // namespace renderer
}  // namespace supra
