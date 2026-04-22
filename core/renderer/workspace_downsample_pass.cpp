#include "workspace_downsample_pass.h"

#ifdef _WIN32

#include "../shader/shader_compiler.h"
#include "../shader/workspace_downsample_shader_contract.h"
#include "../shader/workspace_shader_sources.h"

namespace supra::renderer {

using supra::shader::CompilePixelShader;
using supra::shader::CompileVertexShader;

bool WorkspaceDownsamplePass::Initialize(ID3D11Device* device) {
  Shutdown();

  if (device == nullptr) {
    SetError("Workspace renderer could not create downsample resources without a Direct3D device.");
    return false;
  }

  const auto vertexShaderResult = CompileVertexShader(
      device, supra::shader::WorkspaceFullscreenVertexShaderSource(), "WorkspaceFullscreenVS",
      &fullscreenVs_);
  if (!vertexShaderResult.succeeded) {
    SetError("Workspace renderer could not compile the V2 workspace fullscreen vertex shader: " +
             vertexShaderResult.message);
    return false;
  }

  const auto pixelShaderResult = CompilePixelShader(
      device, supra::shader::WorkspaceBicubicDownsamplePixelShaderSource(),
      "WorkspaceBicubicDownsamplePS", &bicubicDownsamplePs_);
  if (!pixelShaderResult.succeeded) {
    SetError("Workspace renderer could not compile the V2 workspace bicubic downsample shader: " +
             pixelShaderResult.message);
    return false;
  }

  D3D11_SAMPLER_DESC samplerDesc{};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  HRESULT hr = device->CreateSamplerState(&samplerDesc, &sampler_);
  if (FAILED(hr)) {
    SetError("Workspace renderer could not create the V2 workspace downsample sampler state.");
    return false;
  }

  D3D11_BUFFER_DESC bufferDesc{};
  bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bufferDesc.ByteWidth = sizeof(supra::shader::WorkspaceDownsampleConstants);
  bufferDesc.Usage = D3D11_USAGE_DEFAULT;

  hr = device->CreateBuffer(&bufferDesc, nullptr, &constantBuffer_);
  if (FAILED(hr)) {
    SetError("Workspace renderer could not create the V2 workspace downsample constant buffer.");
    return false;
  }

  ClearError();
  return true;
}

void WorkspaceDownsamplePass::Shutdown() {
  constantBuffer_.Reset();
  sampler_.Reset();
  bicubicDownsamplePs_.Reset();
  fullscreenVs_.Reset();
  lastError_.clear();
}

bool WorkspaceDownsamplePass::Render(ID3D11DeviceContext* context,
                                     ID3D11ShaderResourceView* logicalSourceSrv,
                                     unsigned int logicalWidth, unsigned int logicalHeight,
                                     ID3D11RenderTargetView* presentationTargetRtv,
                                     unsigned int presentationWidth,
                                     unsigned int presentationHeight) {
  if (context == nullptr || logicalSourceSrv == nullptr || presentationTargetRtv == nullptr ||
      fullscreenVs_ == nullptr || bicubicDownsamplePs_ == nullptr || sampler_ == nullptr ||
      constantBuffer_ == nullptr) {
    SetError(
        "Workspace renderer does not have the V2 workspace downsample resources required for presentation.");
    return false;
  }

  if (logicalWidth == 0U || logicalHeight == 0U || presentationWidth == 0U ||
      presentationHeight == 0U) {
    SetError("Workspace renderer was asked to downsample an incomplete logical or presentation target.");
    return false;
  }

  const D3D11_VIEWPORT viewport{
      0.0f,
      0.0f,
      static_cast<float>(presentationWidth),
      static_cast<float>(presentationHeight),
      0.0f,
      1.0f,
  };

  const auto constants = supra::shader::BuildWorkspaceDownsampleConstants(
      logicalWidth, logicalHeight, presentationWidth, presentationHeight);
  context->UpdateSubresource(constantBuffer_.Get(), 0, nullptr, &constants, 0, 0);

  ID3D11RenderTargetView* renderTargets[] = {presentationTargetRtv};
  const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  ID3D11ShaderResourceView* sourceResourceViews[] = {logicalSourceSrv};

  context->OMSetRenderTargets(1, renderTargets, nullptr);
  context->ClearRenderTargetView(presentationTargetRtv, clearColor);
  context->RSSetViewports(1, &viewport);
  context->VSSetShader(fullscreenVs_.Get(), nullptr, 0);
  context->PSSetShader(bicubicDownsamplePs_.Get(), nullptr, 0);
  context->PSSetShaderResources(0, 1, sourceResourceViews);
  context->PSSetSamplers(0, 1, sampler_.GetAddressOf());
  context->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
  fullscreenQuad_.Draw(context);

  ID3D11ShaderResourceView* nullShaderResourceViews[] = {nullptr};
  ID3D11RenderTargetView* nullRenderTargets[] = {nullptr};
  context->PSSetShaderResources(0, 1, nullShaderResourceViews);
  context->OMSetRenderTargets(1, nullRenderTargets, nullptr);
  ClearError();
  return true;
}

void WorkspaceDownsamplePass::ClearError() {
  lastError_.clear();
}

void WorkspaceDownsamplePass::SetError(const std::string& errorText) {
  lastError_ = errorText;
}

}  // namespace supra::renderer

#endif
