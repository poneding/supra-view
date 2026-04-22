#pragma once

#include "../core/input/input_mapper.h"

#include <string>

#ifdef _WIN32

#include <windows.h>

namespace supra {
namespace app {

class V2AppController {
 public:
  bool Initialize(HWND windowHandle, UINT width, UINT height);
  void Shutdown();

  bool Tick();
  void OnResize(UINT width, UINT height);
  void OnMouseMove(const supra::input::MouseMoveEvent& mouseMoveEvent);
  void OnMouseButton(const supra::input::MouseButtonEvent& mouseButtonEvent);
  void OnKeyboard(const supra::input::KeyboardEvent& keyboardEvent);

  [[nodiscard]] std::wstring LastErrorText() const;

 private:
  bool HandleShellKeyboardShortcut(const supra::input::KeyboardEvent& keyboardEvent);
  [[nodiscard]] bool HasWindowInputFocus() const;
  void Log(const std::string& message) const;
  void RebuildInputMapping();
  void UpdatePointerFromInputSurface(const supra::input::InputSurfacePoint& inputSurfacePoint);
  void UpdateShellWindowDimensions(UINT width, UINT height);

  supra::input::InputState inputState_{};
  supra::input::InputMapper inputMapper_{};
  supra::input::LogicalWorkspaceSize logicalWorkspaceSize_{};
  HWND windowHandle_ = nullptr;
  UINT shellWindowWidth_ = 0;
  UINT shellWindowHeight_ = 0;
  std::string lastError_;
  bool hadWindowInputFocus_ = false;
  bool consumeBorderlessRestoreEscapeUntilRelease_ = false;
  bool initialized_ = false;
};

}  // namespace app
}  // namespace supra

#else

namespace supra {
namespace app {

class V2AppController {
 public:
  bool Initialize(void*, unsigned int, unsigned int) { return false; }
  void Shutdown() {}
  bool Tick() { return false; }
  void OnResize(unsigned int, unsigned int) {}
  void OnMouseMove(const supra::input::MouseMoveEvent&) {}
  void OnMouseButton(const supra::input::MouseButtonEvent&) {}
  void OnKeyboard(const supra::input::KeyboardEvent&) {}
  [[nodiscard]] std::wstring LastErrorText() const { return L"Windows-only application."; }
};

}  // namespace app
}  // namespace supra

#endif
