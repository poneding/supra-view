#include "workspace_session.h"

namespace supra {
namespace session {

bool WorkspaceSession::Initialize(const WorkspaceSessionConfig& config) {
  Shutdown();
  if (!config.initialResolution.IsValid()) {
    SetError("Workspace session requires both logical and presentation resolutions during initialization.");
    return false;
  }

  if (!workspaceState_.Initialize(config.initialResolution)) {
    SetError("Workspace state rejected the initial workspace resolution.");
    return false;
  }

  if (!workspaceState_.SetDebugOverlayEnabled(config.debugOverlayEnabled) ||
      !workspaceState_.SetDebugTitle(config.sessionName) ||
      !workspaceState_.SetPanelVisibility(config.initialPanelVisibility) ||
      !workspaceState_.SetPanelTitle("Inspector") ||
      !workspaceState_.SetPanelPlaceholderContentEnabled(true)) {
    workspaceState_.Reset();
    SetError("Workspace session could not apply its initial content state.");
    return false;
  }

  elapsedSeconds_ = 0.0f;
  initialized_ = true;
  ClearError();
  return true;
}

void WorkspaceSession::Shutdown() {
  workspaceState_.Reset();
  elapsedSeconds_ = 0.0f;
  initialized_ = false;
  lastError_.clear();
}

bool WorkspaceSession::Update(const WorkspaceUpdateParams& params) {
  if (!initialized_) {
    SetError("Workspace session update was called before initialization.");
    return false;
  }

  elapsedSeconds_ += params.deltaSeconds;

  if (!workspaceState_.Update(params.frameNumber)) {
    SetError("Workspace state update failed.");
    return false;
  }

  ClearError();
  return true;
}

WorkspaceRenderResult WorkspaceSession::Render() const {
  WorkspaceRenderResult result;
  result.state = workspaceState_.BuildRenderState();
  result.hasVisiblePanel =
      result.state.content.panel.visibility == WorkspacePanelVisibility::Visible &&
      result.state.content.panel.placeholderContentEnabled;
  result.hasDebugOverlay = result.state.content.debug.enabled;
  return result;
}

void WorkspaceSession::OnResize(const PresentationWorkspaceSize& presentationSize) {
  if (!initialized_ || presentationSize.IsEmpty()) {
    return;
  }

  workspaceState_.SetPresentationSize(presentationSize);
}

void WorkspaceSession::ClearError() {
  lastError_.clear();
}

void WorkspaceSession::SetError(const std::string& errorText) {
  lastError_ = errorText;
}

}  // namespace session
}  // namespace supra
