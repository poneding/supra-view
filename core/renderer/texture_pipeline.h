#pragma once

#include "../capture/capture_types.h"
#include "renderer_types.h"
#include "../shader/fullscreen_quad.h"

#ifdef _WIN32

#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>

namespace supra::renderer {

class TexturePipeline {
 public:
  bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, const RenderConfig& config);
  void Shutdown();

  bool Resize(UINT outputWidth, UINT outputHeight);
  bool UpdateSource(const supra::capture::CapturedFrame& frame);
  bool Render(ID3D11RenderTargetView* backBufferRtv, UINT backBufferWidth, UINT backBufferHeight);
  bool HasSourceFrame() const noexcept { return hasSourceFrame_; }
  void SetScalingMode(ScalingMode scalingMode) noexcept { scalingMode_ = scalingMode; }

 private:
  bool CreateStates();
  bool CreateShaders();
  bool EnsureSourceResources(const D3D11_TEXTURE2D_DESC& sourceDesc);
  bool EnsureIntermediateResources(UINT width, UINT height, DXGI_FORMAT format);
  bool ExecutePass(ID3D11ShaderResourceView* sourceSrv, UINT sourceWidth, UINT sourceHeight,
                   ID3D11RenderTargetView* targetRtv, UINT targetWidth, UINT targetHeight,
                   ID3D11PixelShader* pixelShader);

  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> sourceTexture_;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sourceSrv_;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> intermediateTexture_;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> intermediateRtv_;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> intermediateSrv_;

  Microsoft::WRL::ComPtr<ID3D11VertexShader> fullscreenVs_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> copyPs_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> bicubicPs_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;

  FullscreenQuad fullscreenQuad_;
  ScalingMode scalingMode_ = ScalingMode::Bicubic;
  UINT outputWidth_ = 0;
  UINT outputHeight_ = 0;
  UINT sourceWidth_ = 0;
  UINT sourceHeight_ = 0;
  DXGI_FORMAT sourceFormat_ = DXGI_FORMAT_UNKNOWN;
  DXGI_FORMAT intermediateFormat_ = DXGI_FORMAT_B8G8R8A8_UNORM;
  bool hasSourceFrame_ = false;
  bool lanczosFallbackLogged_ = false;
};

}  // namespace supra::renderer

#else

namespace supra::renderer {

class TexturePipeline {
 public:
  bool Initialize(void*, void*, const RenderConfig&) { return false; }
  void Shutdown() {}
  bool Resize(unsigned int, unsigned int) { return false; }
  bool UpdateSource(const supra::capture::CapturedFrame&) { return false; }
  bool Render(void*, unsigned int, unsigned int) { return false; }
  bool HasSourceFrame() const noexcept { return false; }
  void SetScalingMode(ScalingMode) noexcept {}
};

}  // namespace supra::renderer

#endif
