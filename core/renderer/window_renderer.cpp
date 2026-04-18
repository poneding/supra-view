#include "window_renderer.h"

#ifdef _WIN32

namespace supra::renderer {

bool WindowRenderer::Initialize(HWND windowHandle, UINT width, UINT height, const RenderConfig& config) {
  config_ = config;
  if (!context_.Initialize(windowHandle, width, height)) {
    return false;
  }

  if (config_.outputWidth == 0) {
    config_.outputWidth = width;
  }
  if (config_.outputHeight == 0) {
    config_.outputHeight = height;
  }

  return pipeline_.Initialize(context_.Device(), context_.Context(), config_);
}

void WindowRenderer::Shutdown() {
  pipeline_.Shutdown();
  context_.Shutdown();
}

bool WindowRenderer::Resize(UINT width, UINT height) {
  if (!context_.Resize(width, height)) {
    return false;
  }

  config_.outputWidth = width;
  config_.outputHeight = height;
  pipeline_.Resize(width, height);

  return true;
}

bool WindowRenderer::RenderCapturedFrame(const supra::capture::CapturedFrame& frame) {
  if (!pipeline_.UpdateSource(frame)) {
    return false;
  }

  if (!pipeline_.Render(context_.BackBufferRtv(), context_.Width(), context_.Height())) {
    return false;
  }

  return context_.Present();
}

bool WindowRenderer::RenderLastFrame() {
  if (!pipeline_.HasSourceFrame()) {
    const float clearColor[4] = {0.05f, 0.05f, 0.08f, 1.0f};
    context_.Clear(clearColor);
    return context_.Present();
  }

  if (!pipeline_.Render(context_.BackBufferRtv(), context_.Width(), context_.Height())) {
    return false;
  }

  return context_.Present();
}

void WindowRenderer::SetWindowMaskRect(const NormalizedMaskRect& rect) noexcept {
  pipeline_.SetWindowMaskRect(rect);
}

void WindowRenderer::ClearWindowMask() noexcept {
  pipeline_.ClearWindowMask();
}

}  // namespace supra::renderer

#endif
