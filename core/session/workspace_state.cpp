#include "workspace_state.h"

namespace supra {
namespace session {

namespace {

std::string FormatSizeString(WorkspaceUint width, WorkspaceUint height) {
  return std::to_string(width) + "x" + std::to_string(height);
}

}  // namespace

bool WorkspaceState::Initialize(const WorkspaceResolution& resolution) {
  Reset();
  if (!resolution.IsValid()) {
    return false;
  }

  resolution_ = resolution;
  initialized_ = true;
  UpdateDerivedState();
  return true;
}

void WorkspaceState::Reset() {
  resolution_ = {};
  content_ = {};
  frameNumber_ = 0;
  presentationScaleX_ = 1.0f;
  presentationScaleY_ = 1.0f;
  initialized_ = false;
}

bool WorkspaceState::Update(std::uint64_t frameNumber) {
  if (!initialized_) {
    return false;
  }

  frameNumber_ = frameNumber;
  UpdateDerivedState();
  return true;
}

bool WorkspaceState::SetPresentationSize(const PresentationWorkspaceSize& presentationSize) {
  if (!initialized_ || presentationSize.IsEmpty()) {
    return false;
  }

  resolution_.presentation = presentationSize;
  UpdateDerivedState();
  return true;
}

bool WorkspaceState::SetDebugOverlayEnabled(bool enabled) {
  if (!initialized_) {
    return false;
  }

  content_.debug.enabled = enabled;
  return true;
}

bool WorkspaceState::SetDebugTitle(const std::string& title) {
  if (!initialized_) {
    return false;
  }

  content_.debug.title = title;
  return true;
}

bool WorkspaceState::SetPanelVisibility(WorkspacePanelVisibility visibility) {
  if (!initialized_) {
    return false;
  }

  content_.panel.visibility = visibility;
  return true;
}

bool WorkspaceState::SetPanelTitle(const std::string& title) {
  if (!initialized_) {
    return false;
  }

  content_.panel.title = title;
  return true;
}

bool WorkspaceState::SetPanelPlaceholderContentEnabled(bool enabled) {
  if (!initialized_) {
    return false;
  }

  content_.panel.placeholderContentEnabled = enabled;
  return true;
}

WorkspaceRenderState WorkspaceState::BuildRenderState() const {
  WorkspaceRenderState renderState;
  renderState.resolution = resolution_;
  renderState.content = content_;
  renderState.presentationScaleX = presentationScaleX_;
  renderState.presentationScaleY = presentationScaleY_;
  renderState.frameNumber = frameNumber_;
  return renderState;
}

void WorkspaceState::UpdateDerivedState() {
  if (!initialized_) {
    return;
  }

  const float logicalWidth = static_cast<float>(resolution_.logical.width);
  const float logicalHeight = static_cast<float>(resolution_.logical.height);
  const float presentationWidth = static_cast<float>(resolution_.presentation.width);
  const float presentationHeight = static_cast<float>(resolution_.presentation.height);

  presentationScaleX_ = presentationWidth / logicalWidth;
  presentationScaleY_ = presentationHeight / logicalHeight;

  content_.debug.summary =
      "Logical " + FormatSizeString(resolution_.logical.width, resolution_.logical.height) +
      " -> presentation " + FormatSizeString(resolution_.presentation.width, resolution_.presentation.height) +
      ", frame " + std::to_string(frameNumber_) + ".";
}

}  // namespace session
}  // namespace supra
