#include "input_mapper.h"

namespace supra {
namespace input {

bool InputMapper::Configure(const InputSurfaceMappingConfig& config) {
  Reset();
  if (!config.IsValid()) {
    return false;
  }

  config_ = config;
  logicalUnitsPerInputSurfacePixelX_ =
      static_cast<float>(config.logicalWorkspaceSize.width) /
      static_cast<float>(config.inputSurfaceSize.width);
  logicalUnitsPerInputSurfacePixelY_ =
      static_cast<float>(config.logicalWorkspaceSize.height) /
      static_cast<float>(config.inputSurfaceSize.height);
  configured_ = true;
  return true;
}

void InputMapper::Reset() {
  config_ = {};
  logicalUnitsPerInputSurfacePixelX_ = 1.0f;
  logicalUnitsPerInputSurfacePixelY_ = 1.0f;
  configured_ = false;
}

MappedWorkspacePoint InputMapper::MapInputSurfaceToLogicalWorkspace(
    const InputSurfacePoint& inputSurfacePosition) const {
  MappedWorkspacePoint mappedWorkspacePoint;
  mappedWorkspacePoint.inputSurfacePosition = inputSurfacePosition;

  if (!configured_) {
    return mappedWorkspacePoint;
  }

  const bool isWithinInputSurface =
      inputSurfacePosition.x >= 0 &&
      static_cast<std::uint32_t>(inputSurfacePosition.x) < config_.inputSurfaceSize.width &&
      inputSurfacePosition.y >= 0 &&
      static_cast<std::uint32_t>(inputSurfacePosition.y) < config_.inputSurfaceSize.height;

  mappedWorkspacePoint.isWithinInputSurface = isWithinInputSurface;
  if (!isWithinInputSurface) {
    return mappedWorkspacePoint;
  }

  mappedWorkspacePoint.logicalWorkspacePosition.x =
      static_cast<float>(inputSurfacePosition.x) * logicalUnitsPerInputSurfacePixelX_;
  mappedWorkspacePoint.logicalWorkspacePosition.y =
      static_cast<float>(inputSurfacePosition.y) * logicalUnitsPerInputSurfacePixelY_;
  mappedWorkspacePoint.isMappedToLogicalWorkspace = true;
  return mappedWorkspacePoint;
}

}  // namespace input
}  // namespace supra
