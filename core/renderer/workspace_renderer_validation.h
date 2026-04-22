#pragma once

#include "../session/workspace_state.h"

namespace supra {
namespace renderer {

[[nodiscard]] bool ValidateWorkspaceResolution(const supra::session::WorkspaceResolution& resolution) noexcept;
[[nodiscard]] bool ValidateWorkspacePresentationSize(unsigned int width, unsigned int height) noexcept;

}  // namespace renderer
}  // namespace supra
