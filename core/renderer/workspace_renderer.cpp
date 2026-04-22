#include "workspace_renderer.h"

#ifdef _WIN32

#include <string>

namespace supra::renderer {

bool WorkspaceRenderer::Initialize(HWND windowHandle, const WorkspaceRendererConfig& config) {
  Shutdown();

  if (!ValidateWorkspaceResolution(config.initialResolution)) {
    SetError("Workspace renderer requires a valid bounded logical and presentation resolution.");
    return false;
  }

  if (!context_.Initialize(windowHandle,
                           config.initialResolution.presentation.width,
                           config.initialResolution.presentation.height)) {
    SetError("Workspace renderer could not initialize the Direct3D context.");
    const std::string errorText = lastError_;
    Shutdown();
    lastError_ = errorText;
    return false;
  }

  if (context_.Context() == nullptr) {
    SetError("Workspace renderer did not receive a valid Direct3D immediate context.");
    const std::string errorText = lastError_;
    Shutdown();
    lastError_ = errorText;
    return false;
  }

  HRESULT hr = context_.Context()->QueryInterface(IID_PPV_ARGS(context1_.ReleaseAndGetAddressOf()));
  if (FAILED(hr) || context1_ == nullptr) {
    SetError("Workspace renderer requires an ID3D11DeviceContext1 immediate context for placeholder rendering.");
    const std::string errorText = lastError_;
    Shutdown();
    lastError_ = errorText;
    return false;
  }

  resolution_ = config.initialResolution;

  if (!EnsureTargetsForResolution(resolution_)) {
    const std::string errorText = lastError_;
    Shutdown();
    lastError_ = errorText;
    return false;
  }

  if (!downsamplePass_.Initialize(context_.Device())) {
    SetError(downsamplePass_.LastError());
    const std::string errorText = lastError_;
    Shutdown();
    lastError_ = errorText;
    return false;
  }

  initialized_ = true;
  ClearError();
  return true;
}

void WorkspaceRenderer::Shutdown() {
  downsamplePass_.Shutdown();
  presentationTarget_.Reset();
  logicalTarget_.Reset();
  resolution_ = {};
  context1_.Reset();
  context_.Shutdown();
  lastError_.clear();
  initialized_ = false;
}

bool WorkspaceRenderer::ResizePresentation(UINT width, UINT height) {
  if (!initialized_ || !ValidateWorkspacePresentationSize(width, height)) {
    SetError("Workspace renderer resize requires a bounded non-zero presentation size after initialization.");
    return false;
  }

  if (!context_.Resize(width, height)) {
    SetError("Workspace renderer could not resize swap-chain resources.");
    return false;
  }

  resolution_.presentation.width = width;
  resolution_.presentation.height = height;
  presentationTarget_.Reset();
  ClearError();
  return true;
}

bool WorkspaceRenderer::Render(const supra::session::WorkspaceRenderResult& renderResult) {
  if (!initialized_) {
    SetError("Workspace renderer render was called before initialization.");
    return false;
  }

  if (!ValidateWorkspaceResolution(renderResult.state.resolution)) {
    SetError("Workspace renderer received an invalid or unbounded workspace resolution.");
    return false;
  }

  const auto& renderResolution = renderResult.state.resolution;
  if (context_.Width() != renderResolution.presentation.width ||
      context_.Height() != renderResolution.presentation.height) {
    if (!ResizePresentation(renderResolution.presentation.width,
                            renderResolution.presentation.height)) {
      return false;
    }
  }

  resolution_ = renderResolution;
  if (!EnsureTargetsForResolution(resolution_)) {
    return false;
  }

  PlaceholderScene scene;
  if (!RenderLogicalPass(renderResult, scene) || !RenderPresentationPass()) {
    return false;
  }

  if (context_.Context() == nullptr || context_.BackBuffer() == nullptr ||
      presentationTarget_.texture == nullptr) {
    SetError("Workspace renderer could not resolve the presentation target into the swap-chain backbuffer.");
    return false;
  }

  context_.Context()->CopyResource(context_.BackBuffer(), presentationTarget_.texture.Get());
  ClearError();
  if (!context_.Present()) {
    SetError("Workspace renderer failed to present the presentation target.");
    return false;
  }

  return true;
}

bool WorkspaceRenderer::EnsureTargetsForResolution(const supra::session::WorkspaceResolution& resolution) {
  if (!ValidateWorkspaceResolution(resolution)) {
    SetError("Workspace renderer requires bounded logical and presentation targets.");
    return false;
  }

  const DXGI_FORMAT targetFormat =
      context_.BackBufferFormat() == DXGI_FORMAT_UNKNOWN ? DXGI_FORMAT_B8G8R8A8_UNORM
                                                         : context_.BackBufferFormat();

  if (!EnsureRenderTarget(logicalTarget_, resolution.logical.width, resolution.logical.height,
                          targetFormat)) {
    SetError("Workspace renderer could not create the logical offscreen target.");
    return false;
  }

  if (!EnsureRenderTarget(presentationTarget_, resolution.presentation.width,
                          resolution.presentation.height, targetFormat)) {
    SetError("Workspace renderer could not create the presentation target.");
    return false;
  }

  return true;
}

bool WorkspaceRenderer::EnsureRenderTarget(RenderTargetResources& target, UINT width, UINT height,
                                           DXGI_FORMAT format) {
  if (width == 0 || height == 0 || context_.Device() == nullptr) {
    return false;
  }

  if (target.texture != nullptr && target.width == width && target.height == height &&
      target.format == format) {
    return true;
  }

  target.Reset();

  D3D11_TEXTURE2D_DESC textureDesc{};
  textureDesc.Width = width;
  textureDesc.Height = height;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = format;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

  HRESULT hr = context_.Device()->CreateTexture2D(&textureDesc, nullptr, &target.texture);
  if (FAILED(hr)) {
    return false;
  }

  hr = context_.Device()->CreateRenderTargetView(target.texture.Get(), nullptr, &target.rtv);
  if (FAILED(hr)) {
    target.Reset();
    return false;
  }

  hr = context_.Device()->CreateShaderResourceView(target.texture.Get(), nullptr, &target.srv);
  if (FAILED(hr)) {
    target.Reset();
    return false;
  }

  target.width = width;
  target.height = height;
  target.format = format;
  return true;
}

bool WorkspaceRenderer::RenderLogicalPass(const supra::session::WorkspaceRenderResult& renderResult,
                                          PlaceholderScene& scene) {
  scene = BuildPlaceholderScene(resolution_.logical.width, resolution_.logical.height, renderResult);
  return RenderPlaceholderScene(logicalTarget_.rtv.Get(), resolution_.logical.width,
                                resolution_.logical.height, scene);
}

bool WorkspaceRenderer::RenderPresentationPass() {
  if (downsamplePass_.Render(context_.Context(), logicalTarget_.srv.Get(), resolution_.logical.width,
                             resolution_.logical.height, presentationTarget_.rtv.Get(),
                             resolution_.presentation.width, resolution_.presentation.height)) {
    return true;
  }

  SetError(downsamplePass_.LastError());
  return false;
}

bool WorkspaceRenderer::RenderPlaceholderScene(
    ID3D11RenderTargetView* renderTargetView, UINT width, UINT height,
    const PlaceholderScene& scene) {
  if (renderTargetView == nullptr || width == 0 || height == 0 || context_.Context() == nullptr) {
    SetError("Workspace renderer was asked to draw into an incomplete render target.");
    return false;
  }

  ID3D11RenderTargetView* renderTargets[] = {renderTargetView};
  const D3D11_VIEWPORT viewport{
      0.0f,
      0.0f,
      static_cast<float>(width),
      static_cast<float>(height),
      0.0f,
      1.0f,
  };

  context_.Context()->OMSetRenderTargets(1, renderTargets, nullptr);
  context_.Context()->RSSetViewports(1, &viewport);
  context_.Context()->ClearRenderTargetView(renderTargetView, scene.clearColor.data());
  DrawPlaceholderScene(context1_.Get(), renderTargetView, width, height, scene);

  ID3D11RenderTargetView* nullRenderTargets[] = {nullptr};
  context_.Context()->OMSetRenderTargets(1, nullRenderTargets, nullptr);
  return true;
}

void WorkspaceRenderer::ClearError() {
  lastError_.clear();
}

void WorkspaceRenderer::SetError(const std::string& errorText) {
  lastError_ = errorText;
}

}  // namespace supra::renderer

#endif
