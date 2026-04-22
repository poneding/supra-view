#pragma once

#include "d3d_context.h"
#include "workspace_downsample_pass.h"
#include "workspace_renderer_validation.h"
#include "../session/workspace_session.h"

#include <string>

#ifdef _WIN32

#include <windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <wrl/client.h>

#include "workspace_placeholder_scene.h"

namespace supra {
namespace renderer {

struct WorkspaceRendererConfig {
  supra::session::WorkspaceResolution initialResolution{};
};

class WorkspaceRenderer {
 public:
  bool Initialize(HWND windowHandle, const WorkspaceRendererConfig& config);
  void Shutdown();

  bool ResizePresentation(UINT width, UINT height);
  bool Render(const supra::session::WorkspaceRenderResult& renderResult);

  [[nodiscard]] const supra::session::WorkspaceResolution& Resolution() const noexcept { return resolution_; }
  [[nodiscard]] ID3D11Texture2D* LogicalTargetTexture() const noexcept { return logicalTarget_.texture.Get(); }
  [[nodiscard]] ID3D11RenderTargetView* LogicalTargetRtv() const noexcept { return logicalTarget_.rtv.Get(); }
  [[nodiscard]] ID3D11ShaderResourceView* LogicalTargetSrv() const noexcept { return logicalTarget_.srv.Get(); }
  [[nodiscard]] ID3D11Texture2D* PresentationTargetTexture() const noexcept { return presentationTarget_.texture.Get(); }
  [[nodiscard]] ID3D11RenderTargetView* PresentationTargetRtv() const noexcept { return presentationTarget_.rtv.Get(); }
  [[nodiscard]] ID3D11ShaderResourceView* PresentationTargetSrv() const noexcept { return presentationTarget_.srv.Get(); }
  [[nodiscard]] D3DContext& Context() noexcept { return context_; }
  [[nodiscard]] const std::string& LastError() const noexcept { return lastError_; }

 private:
  struct RenderTargetResources {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    UINT width = 0;
    UINT height = 0;
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

    void Reset() noexcept {
      srv.Reset();
      rtv.Reset();
      texture.Reset();
      width = 0;
      height = 0;
      format = DXGI_FORMAT_UNKNOWN;
    }
  };

  bool EnsureTargetsForResolution(const supra::session::WorkspaceResolution& resolution);
  bool EnsureRenderTarget(RenderTargetResources& target, UINT width, UINT height, DXGI_FORMAT format);
  bool RenderLogicalPass(const supra::session::WorkspaceRenderResult& renderResult,
                         PlaceholderScene& scene);
  bool RenderPresentationPass();
  bool RenderPlaceholderScene(ID3D11RenderTargetView* renderTargetView, UINT width, UINT height,
                              const PlaceholderScene& scene);
  void ClearError();
  void SetError(const std::string& errorText);

  D3DContext context_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context1_;
  supra::session::WorkspaceResolution resolution_{};
  RenderTargetResources logicalTarget_{};
  RenderTargetResources presentationTarget_{};
  WorkspaceDownsamplePass downsamplePass_{};
  std::string lastError_{};
  bool initialized_ = false;
};

}  // namespace renderer
}  // namespace supra

#else

namespace supra {
namespace renderer {

struct WorkspaceRendererConfig {
  supra::session::WorkspaceResolution initialResolution{};
};

class WorkspaceRenderer {
 public:
  bool Initialize(void*, const WorkspaceRendererConfig&) { return false; }
  void Shutdown() {}
  bool ResizePresentation(unsigned int, unsigned int) { return false; }
  bool Render(const supra::session::WorkspaceRenderResult&) { return false; }
  [[nodiscard]] const supra::session::WorkspaceResolution& Resolution() const noexcept { return resolution_; }
  [[nodiscard]] void* LogicalTargetTexture() const noexcept { return nullptr; }
  [[nodiscard]] void* LogicalTargetRtv() const noexcept { return nullptr; }
  [[nodiscard]] void* LogicalTargetSrv() const noexcept { return nullptr; }
  [[nodiscard]] void* PresentationTargetTexture() const noexcept { return nullptr; }
  [[nodiscard]] void* PresentationTargetRtv() const noexcept { return nullptr; }
  [[nodiscard]] void* PresentationTargetSrv() const noexcept { return nullptr; }
  [[nodiscard]] D3DContext& Context() noexcept { return context_; }
  [[nodiscard]] const std::string& LastError() const noexcept { return lastError_; }

 private:
  D3DContext context_;
  supra::session::WorkspaceResolution resolution_{};
  std::string lastError_{};
};

}  // namespace renderer
}  // namespace supra

#endif
