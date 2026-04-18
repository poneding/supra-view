#include "app_controller.h"

#ifdef _WIN32

#include "../core/capture/capture_utils.h"

namespace supra::app {

bool AppController::Initialize(HWND windowHandle, UINT width, UINT height) {
  renderConfig_.outputWidth = width;
  renderConfig_.outputHeight = height;
  renderConfig_.scalingMode = supra::renderer::ScalingMode::Bicubic;
  captureConfig_.outputIndex = 0;
  captureConfig_.timeoutMilliseconds = 8;

  if (!renderer_.Initialize(windowHandle, width, height, renderConfig_)) {
    lastError_ = "Failed to initialize the D3D11 renderer.";
    return false;
  }

  if (!capture_.Initialize(renderer_.Device(), captureConfig_)) {
    lastError_ = "Failed to initialize desktop capture. DuplicateOutput may be unavailable in this session.";
    renderer_.Shutdown();
    return false;
  }

  Log("Renderer adapter: " + renderer_.Context().AdapterDescription());
  Log("Capture output: " + capture_.CurrentOutputName());
  initialized_ = true;
  return true;
}

void AppController::Shutdown() {
  capture_.Shutdown();
  renderer_.Shutdown();
  initialized_ = false;
}

bool AppController::Tick() {
  if (!initialized_) {
    return false;
  }

  const supra::capture::CaptureResult result = capture_.AcquireFrame();
  switch (result.status) {
    case supra::capture::CaptureStatus::FrameAvailable: {
      const bool renderOk = renderer_.RenderCapturedFrame(result.frame);
      capture_.ReleaseFrame();
      return renderOk;
    }
    case supra::capture::CaptureStatus::Timeout:
      return renderer_.RenderLastFrame();
    case supra::capture::CaptureStatus::AccessLost:
      Log("Desktop duplication access lost. Attempting to recreate capture.");
      return RecoverDuplication() && renderer_.RenderLastFrame();
    case supra::capture::CaptureStatus::Error:
    default:
      lastError_ = result.message.empty() ? "Desktop capture failed." : result.message;
      Log(lastError_);
      return renderer_.RenderLastFrame();
  }
}

void AppController::OnResize(UINT width, UINT height) {
  if (!initialized_ || width == 0 || height == 0) {
    return;
  }

  renderer_.Resize(width, height);
}

std::wstring AppController::LastErrorText() const {
  if (lastError_.empty()) {
    return L"Unknown error.";
  }

  const int length = MultiByteToWideChar(CP_UTF8, 0, lastError_.c_str(), -1, nullptr, 0);
  std::wstring result(static_cast<std::size_t>(length), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, lastError_.c_str(), -1, result.data(), length);
  if (!result.empty() && result.back() == L'\0') {
    result.pop_back();
  }
  return result;
}

void AppController::Log(const std::string& message) const {
  if (message.empty()) {
    return;
  }

  OutputDebugStringA(message.c_str());
  OutputDebugStringA("\n");
}

bool AppController::RecoverDuplication() {
  capture_.ReleaseFrame();
  if (!capture_.Reinitialize()) {
    lastError_ = "Failed to recreate desktop duplication after access loss.";
    return false;
  }
  return true;
}

}  // namespace supra::app

#endif
