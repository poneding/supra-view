#include "texture_pipeline.h"

#ifdef _WIN32

#include "../shader/shader_compiler.h"
#include "../shader/shader_sources.h"

#include <string>

namespace supra::renderer {

using Microsoft::WRL::ComPtr;
using supra::shader::CompilePixelShader;
using supra::shader::CompileVertexShader;

namespace {

void LogDebugMessage(const std::string& message) {
  if (message.empty()) {
    return;
  }

  OutputDebugStringA(message.c_str());
  OutputDebugStringA("\n");
}

}  // namespace

bool TexturePipeline::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, const RenderConfig& config) {
  Shutdown();

  if (device == nullptr || context == nullptr) {
    return false;
  }

  device_ = device;
  context_ = context;
  outputWidth_ = config.outputWidth;
  outputHeight_ = config.outputHeight;
  scalingMode_ = config.scalingMode;

  return CreateStates() && CreateShaders();
}

void TexturePipeline::Shutdown() {
  constantBuffer_.Reset();
  sampler_.Reset();
  bicubicPs_.Reset();
  copyPs_.Reset();
  fullscreenVs_.Reset();
  intermediateSrv_.Reset();
  intermediateRtv_.Reset();
  intermediateTexture_.Reset();
  sourceSrv_.Reset();
  sourceTexture_.Reset();
  context_.Reset();
  device_.Reset();
  outputWidth_ = 0;
  outputHeight_ = 0;
  sourceWidth_ = 0;
  sourceHeight_ = 0;
  sourceFormat_ = DXGI_FORMAT_UNKNOWN;
  hasSourceFrame_ = false;
  lanczosFallbackLogged_ = false;
}

bool TexturePipeline::CreateStates() {
  D3D11_SAMPLER_DESC samplerDesc{};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.MaxLOD = 0.0f;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

  HRESULT hr = device_->CreateSamplerState(&samplerDesc, &sampler_);
  if (FAILED(hr)) {
    return false;
  }

  D3D11_BUFFER_DESC bufferDesc{};
  bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bufferDesc.ByteWidth = sizeof(DownsampleConstants);
  bufferDesc.Usage = D3D11_USAGE_DEFAULT;

  hr = device_->CreateBuffer(&bufferDesc, nullptr, &constantBuffer_);
  return SUCCEEDED(hr);
}

bool TexturePipeline::CreateShaders() {
  const auto vsResult = CompileVertexShader(device_.Get(), supra::shader::FullscreenVertexShaderSource(), "FullscreenVS", &fullscreenVs_);
  if (!vsResult.succeeded) {
    LogDebugMessage(vsResult.message);
    return false;
  }

  const auto copyResult = CompilePixelShader(device_.Get(), supra::shader::CopyPixelShaderSource(), "CopyPS", &copyPs_);
  if (!copyResult.succeeded) {
    LogDebugMessage(copyResult.message);
    return false;
  }

  const auto bicubicResult = CompilePixelShader(device_.Get(), supra::shader::BicubicPixelShaderSource(), "BicubicPS", &bicubicPs_);
  if (!bicubicResult.succeeded) {
    LogDebugMessage(bicubicResult.message);
    return false;
  }

  return true;
}

bool TexturePipeline::Resize(UINT outputWidth, UINT outputHeight) {
  outputWidth_ = outputWidth;
  outputHeight_ = outputHeight;
  intermediateSrv_.Reset();
  intermediateRtv_.Reset();
  intermediateTexture_.Reset();
  return true;
}

void TexturePipeline::SetWindowMaskRect(const NormalizedMaskRect& rect) noexcept {
  maskRect_ = rect;
  hasMaskRect_ = true;
}

void TexturePipeline::ClearWindowMask() noexcept {
  maskRect_ = NormalizedMaskRect{};
  hasMaskRect_ = false;
}

bool TexturePipeline::EnsureSourceResources(const D3D11_TEXTURE2D_DESC& sourceDesc) {
  if (sourceTexture_ != nullptr && sourceWidth_ == sourceDesc.Width && sourceHeight_ == sourceDesc.Height && sourceFormat_ == sourceDesc.Format) {
    return true;
  }

  sourceSrv_.Reset();
  sourceTexture_.Reset();

  D3D11_TEXTURE2D_DESC copyDesc = sourceDesc;
  copyDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  copyDesc.Usage = D3D11_USAGE_DEFAULT;
  copyDesc.CPUAccessFlags = 0;
  copyDesc.MiscFlags = 0;
  copyDesc.MipLevels = 1;
  copyDesc.ArraySize = 1;
  copyDesc.SampleDesc.Count = 1;
  copyDesc.SampleDesc.Quality = 0;

  HRESULT hr = device_->CreateTexture2D(&copyDesc, nullptr, &sourceTexture_);
  if (FAILED(hr)) {
    return false;
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
  srvDesc.Format = copyDesc.Format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;
  hr = device_->CreateShaderResourceView(sourceTexture_.Get(), &srvDesc, &sourceSrv_);
  if (FAILED(hr)) {
    return false;
  }

  sourceWidth_ = copyDesc.Width;
  sourceHeight_ = copyDesc.Height;
  sourceFormat_ = copyDesc.Format;
  return true;
}

bool TexturePipeline::EnsureIntermediateResources(UINT width, UINT height, DXGI_FORMAT format) {
  if (intermediateTexture_ != nullptr && width == outputWidth_ && height == outputHeight_ && format == intermediateFormat_) {
    return true;
  }

  intermediateSrv_.Reset();
  intermediateRtv_.Reset();
  intermediateTexture_.Reset();

  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

  HRESULT hr = device_->CreateTexture2D(&desc, nullptr, &intermediateTexture_);
  if (FAILED(hr)) {
    return false;
  }

  hr = device_->CreateRenderTargetView(intermediateTexture_.Get(), nullptr, &intermediateRtv_);
  if (FAILED(hr)) {
    return false;
  }

  hr = device_->CreateShaderResourceView(intermediateTexture_.Get(), nullptr, &intermediateSrv_);
  if (FAILED(hr)) {
    return false;
  }

  intermediateFormat_ = format;
  return true;
}

bool TexturePipeline::UpdateSource(const supra::capture::CapturedFrame& frame) {
  if (!frame.IsValid()) {
    return false;
  }

  D3D11_TEXTURE2D_DESC desc{};
  frame.texture->GetDesc(&desc);
  if (!EnsureSourceResources(desc)) {
    return false;
  }

  context_->CopyResource(sourceTexture_.Get(), frame.texture.Get());
  hasSourceFrame_ = true;
  return true;
}

bool TexturePipeline::ExecutePass(ID3D11ShaderResourceView* sourceSrv, UINT sourceWidth, UINT sourceHeight,
                                  ID3D11RenderTargetView* targetRtv, UINT targetWidth, UINT targetHeight,
                                  ID3D11PixelShader* pixelShader) {
  if (sourceSrv == nullptr || targetRtv == nullptr || pixelShader == nullptr) {
    return false;
  }

  const D3D11_VIEWPORT viewport{
      0.0f,
      0.0f,
      static_cast<float>(targetWidth),
      static_cast<float>(targetHeight),
      0.0f,
      1.0f,
  };

  DownsampleConstants constants{};
  constants.sourceSize[0] = static_cast<float>(sourceWidth);
  constants.sourceSize[1] = static_cast<float>(sourceHeight);
  constants.inverseSourceSize[0] = sourceWidth > 0 ? 1.0f / static_cast<float>(sourceWidth) : 0.0f;
  constants.inverseSourceSize[1] = sourceHeight > 0 ? 1.0f / static_cast<float>(sourceHeight) : 0.0f;
  constants.targetSize[0] = static_cast<float>(targetWidth);
  constants.targetSize[1] = static_cast<float>(targetHeight);
  constants.filterMode = static_cast<float>(scalingMode_ == ScalingMode::Lanczos ? 1 : 0);

  if (hasMaskRect_ && sourceWidth > 0 && sourceHeight > 0) {
    constants.maskTopLeft[0] = static_cast<float>(maskRect_.left) / static_cast<float>(sourceWidth);
    constants.maskTopLeft[1] = static_cast<float>(maskRect_.top) / static_cast<float>(sourceHeight);
    constants.maskBottomRight[0] = static_cast<float>(maskRect_.right) / static_cast<float>(sourceWidth);
    constants.maskBottomRight[1] = static_cast<float>(maskRect_.bottom) / static_cast<float>(sourceHeight);
    constants.maskEnabled = 1.0f;
  }

  context_->UpdateSubresource(constantBuffer_.Get(), 0, nullptr, &constants, 0, 0);

  const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  context_->OMSetRenderTargets(1, &targetRtv, nullptr);
  context_->ClearRenderTargetView(targetRtv, clearColor);
  context_->RSSetViewports(1, &viewport);
  context_->VSSetShader(fullscreenVs_.Get(), nullptr, 0);
  context_->PSSetShader(pixelShader, nullptr, 0);
  context_->PSSetShaderResources(0, 1, &sourceSrv);
  context_->PSSetSamplers(0, 1, sampler_.GetAddressOf());
  context_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
  fullscreenQuad_.Draw(context_.Get());

  ID3D11ShaderResourceView* nullSrv = nullptr;
  context_->PSSetShaderResources(0, 1, &nullSrv);
  return true;
}

bool TexturePipeline::Render(ID3D11RenderTargetView* backBufferRtv, UINT backBufferWidth, UINT backBufferHeight) {
  if (!hasSourceFrame_ || backBufferRtv == nullptr || outputWidth_ == 0 || outputHeight_ == 0) {
    return false;
  }

  if (!EnsureIntermediateResources(outputWidth_, outputHeight_, sourceFormat_ == DXGI_FORMAT_UNKNOWN ? DXGI_FORMAT_B8G8R8A8_UNORM : sourceFormat_)) {
    return false;
  }

  ID3D11PixelShader* filterShader = bicubicPs_.Get();
  if (scalingMode_ == ScalingMode::Lanczos && !lanczosFallbackLogged_) {
    LogDebugMessage("Lanczos mode is not implemented yet. Falling back to bicubic.");
    lanczosFallbackLogged_ = true;
  }

  if (!ExecutePass(sourceSrv_.Get(), sourceWidth_, sourceHeight_, intermediateRtv_.Get(), outputWidth_, outputHeight_, filterShader)) {
    return false;
  }

  return ExecutePass(intermediateSrv_.Get(), outputWidth_, outputHeight_, backBufferRtv, backBufferWidth, backBufferHeight, copyPs_.Get());
}

}  // namespace supra::renderer

#endif
