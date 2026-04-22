#pragma once

#include <string>

#ifdef _WIN32

#include <d3d11.h>
#include <wrl/client.h>

#include "../shader/fullscreen_quad.h"

namespace supra {
namespace renderer {

class WorkspaceDownsamplePass {
 public:
  bool Initialize(ID3D11Device* device);
  void Shutdown();

  bool Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView* logicalSourceSrv,
              unsigned int logicalWidth, unsigned int logicalHeight,
              ID3D11RenderTargetView* presentationTargetRtv, unsigned int presentationWidth,
              unsigned int presentationHeight);

  [[nodiscard]] const std::string& LastError() const noexcept { return lastError_; }

 private:
  void ClearError();
  void SetError(const std::string& errorText);

  Microsoft::WRL::ComPtr<ID3D11VertexShader> fullscreenVs_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> bicubicDownsamplePs_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;
  FullscreenQuad fullscreenQuad_{};
  std::string lastError_{};
};

}  // namespace renderer
}  // namespace supra

#else

namespace supra {
namespace renderer {

class WorkspaceDownsamplePass {
 public:
  bool Initialize(void*) { return false; }
  void Shutdown() {}
  bool Render(void*, void*, unsigned int, unsigned int, void*, unsigned int, unsigned int) {
    return false;
  }
  [[nodiscard]] const std::string& LastError() const noexcept { return lastError_; }

 private:
  std::string lastError_{};
};

}  // namespace renderer
}  // namespace supra

#endif
