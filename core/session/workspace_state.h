#pragma once

#include <cstdint>
#include <string>

#ifdef _WIN32
#include <windows.h>
using WorkspaceUint = UINT;
#else
using WorkspaceUint = std::uint32_t;
#endif

namespace supra {
namespace session {

struct LogicalWorkspaceSize {
  WorkspaceUint width = 0;
  WorkspaceUint height = 0;

  [[nodiscard]] bool IsEmpty() const noexcept { return width == 0 || height == 0; }
};

struct PresentationWorkspaceSize {
  WorkspaceUint width = 0;
  WorkspaceUint height = 0;

  [[nodiscard]] bool IsEmpty() const noexcept { return width == 0 || height == 0; }
};

struct WorkspaceResolution {
  LogicalWorkspaceSize logical{};
  PresentationWorkspaceSize presentation{};

  [[nodiscard]] bool IsValid() const noexcept {
    return !logical.IsEmpty() && !presentation.IsEmpty();
  }
};

enum class WorkspacePanelVisibility : std::uint8_t {
  Hidden = 0,
  Visible = 1,
};

struct WorkspaceBackgroundState {
  float red = 0.08f;
  float green = 0.08f;
  float blue = 0.11f;
  float alpha = 1.0f;
};

struct WorkspaceDebugState {
  bool enabled = true;
  std::string title = "Supra View V2";
  std::string summary = "Application-owned workspace placeholder content.";
};

struct WorkspacePanelState {
  WorkspacePanelVisibility visibility = WorkspacePanelVisibility::Visible;
  std::string title = "Inspector";
  bool placeholderContentEnabled = true;
};

struct WorkspaceContentState {
  WorkspaceBackgroundState background{};
  WorkspaceDebugState debug{};
  WorkspacePanelState panel{};
};

struct WorkspaceRenderState {
  WorkspaceResolution resolution{};
  WorkspaceContentState content{};
  float presentationScaleX = 1.0f;
  float presentationScaleY = 1.0f;
  std::uint64_t frameNumber = 0;
};

class WorkspaceState {
 public:
  bool Initialize(const WorkspaceResolution& resolution);
  void Reset();

  bool Update(std::uint64_t frameNumber);
  bool SetPresentationSize(const PresentationWorkspaceSize& presentationSize);
  bool SetDebugOverlayEnabled(bool enabled);
  bool SetDebugTitle(const std::string& title);
  bool SetPanelVisibility(WorkspacePanelVisibility visibility);
  bool SetPanelTitle(const std::string& title);
  bool SetPanelPlaceholderContentEnabled(bool enabled);

  [[nodiscard]] WorkspaceRenderState BuildRenderState() const;
  [[nodiscard]] const WorkspaceResolution& Resolution() const noexcept { return resolution_; }
  [[nodiscard]] const WorkspaceContentState& Content() const noexcept { return content_; }
  [[nodiscard]] bool IsInitialized() const noexcept { return initialized_; }

 private:
  void UpdateDerivedState();

  WorkspaceResolution resolution_{};
  WorkspaceContentState content_{};
  std::uint64_t frameNumber_ = 0;
  float presentationScaleX_ = 1.0f;
  float presentationScaleY_ = 1.0f;
  bool initialized_ = false;
};

}  // namespace session
}  // namespace supra
