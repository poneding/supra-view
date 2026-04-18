#pragma once

#include "../capture/capture_types.h"
#include "d3d_context.h"
#include "renderer_types.h"
#include "texture_pipeline.h"

#ifdef _WIN32

#include <windows.h>

namespace supra::renderer {

class WindowRenderer {
 public:
  bool Initialize(HWND windowHandle, UINT width, UINT height, const RenderConfig& config);
  void Shutdown();

  bool Resize(UINT width, UINT height);
  bool RenderCapturedFrame(const supra::capture::CapturedFrame& frame);
  bool RenderLastFrame();

  [[nodiscard]] ID3D11Device* Device() const noexcept { return context_.Device(); }
  [[nodiscard]] D3DContext& Context() noexcept { return context_; }

 private:
  D3DContext context_;
  TexturePipeline pipeline_;
  RenderConfig config_{};
};

}  // namespace supra::renderer

#else

namespace supra::renderer {

class WindowRenderer {
 public:
  bool Initialize(void*, unsigned int, unsigned int, const RenderConfig&) { return false; }
  void Shutdown() {}
  bool Resize(unsigned int, unsigned int) { return false; }
  bool RenderCapturedFrame(const supra::capture::CapturedFrame&) { return false; }
  bool RenderLastFrame() { return false; }
  [[nodiscard]] void* Device() const noexcept { return nullptr; }
  [[nodiscard]] D3DContext& Context() noexcept { return context_; }

 private:
  D3DContext context_;
};

}  // namespace supra::renderer

#endif
