#pragma once

#include "../core/capture/desktop_capture.h"
#include "../core/renderer/window_renderer.h"

#include <string>

#ifdef _WIN32

#include <windows.h>

namespace supra::app {

class AppController {
 public:
  bool Initialize(HWND windowHandle, UINT width, UINT height);
  void Shutdown();

  bool Tick();
  void OnResize(UINT width, UINT height);

  [[nodiscard]] std::wstring LastErrorText() const;

 private:
  void Log(const std::string& message) const;
  bool RecoverDuplication();

  supra::renderer::WindowRenderer renderer_;
  supra::capture::DesktopCapture capture_;
  supra::renderer::RenderConfig renderConfig_{};
  supra::capture::CaptureConfig captureConfig_{};
  std::string lastError_;
  bool initialized_ = false;
};

}  // namespace supra::app

#else

namespace supra::app {

class AppController {
 public:
  bool Initialize(void*, unsigned int, unsigned int) { return false; }
  void Shutdown() {}
  bool Tick() { return false; }
  void OnResize(unsigned int, unsigned int) {}
  [[nodiscard]] std::wstring LastErrorText() const { return L"Windows-only application."; }
};

}  // namespace supra::app

#endif
