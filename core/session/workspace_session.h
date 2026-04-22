#pragma once

#include "workspace_state.h"

#include <string>

namespace supra {
namespace session {

struct WorkspaceSessionConfig {
  WorkspaceResolution initialResolution{};
  bool debugOverlayEnabled = true;
  WorkspacePanelVisibility initialPanelVisibility = WorkspacePanelVisibility::Visible;
  std::string sessionName = "workspace-session";
};

struct WorkspaceUpdateParams {
  float deltaSeconds = 0.0f;
  std::uint64_t frameNumber = 0;
};

struct WorkspaceRenderResult {
  WorkspaceRenderState state{};
  bool hasVisiblePanel = false;
  bool hasDebugOverlay = false;
};

class WorkspaceSession {
 public:
  bool Initialize(const WorkspaceSessionConfig& config);
  void Shutdown();

  bool Update(const WorkspaceUpdateParams& params);
  [[nodiscard]] WorkspaceRenderResult Render() const;
  void OnResize(const PresentationWorkspaceSize& presentationSize);

  [[nodiscard]] const WorkspaceState& State() const noexcept { return workspaceState_; }
  [[nodiscard]] bool IsInitialized() const noexcept { return initialized_; }
  [[nodiscard]] const std::string& LastError() const noexcept { return lastError_; }

 private:
  void ClearError();
  void SetError(const std::string& errorText);

  WorkspaceState workspaceState_{};
  float elapsedSeconds_ = 0.0f;
  bool initialized_ = false;
  std::string lastError_{};
};

}  // namespace session
}  // namespace supra
