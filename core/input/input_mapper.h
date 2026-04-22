#pragma once

#include "input_state.h"

namespace supra {
namespace input {

struct InputSurfaceMappingConfig {
  InputSurfaceSize inputSurfaceSize{};
  LogicalWorkspaceSize logicalWorkspaceSize{};

  [[nodiscard]] bool IsValid() const noexcept {
    return !inputSurfaceSize.IsEmpty() && !logicalWorkspaceSize.IsEmpty();
  }
};

class InputMapper {
 public:
  bool Configure(const InputSurfaceMappingConfig& config);
  void Reset();

  [[nodiscard]] bool IsConfigured() const noexcept { return configured_; }
  [[nodiscard]] const InputSurfaceMappingConfig& Config() const noexcept { return config_; }
  [[nodiscard]] MappedWorkspacePoint MapInputSurfaceToLogicalWorkspace(
      const InputSurfacePoint& inputSurfacePosition) const;

 private:
  InputSurfaceMappingConfig config_{};
  float logicalUnitsPerInputSurfacePixelX_ = 1.0f;
  float logicalUnitsPerInputSurfacePixelY_ = 1.0f;
  bool configured_ = false;
};

}  // namespace input
}  // namespace supra
