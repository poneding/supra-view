#include "d3d_context.h"

#ifdef _WIN32

#include "../capture/capture_utils.h"

#include <iterator>

namespace supra::renderer {

using Microsoft::WRL::ComPtr;

bool D3DContext::Initialize(HWND windowHandle, UINT width, UINT height) {
  Shutdown();

  windowHandle_ = windowHandle;
  width_ = width;
  height_ = height;

  if (!SelectAdapter()) {
    return false;
  }

  if (!CreateDeviceAndSwapChain()) {
    return false;
  }

  return CreateBackBufferResources();
}

void D3DContext::Shutdown() {
  if (context_ != nullptr) {
    context_->ClearState();
  }

  backBufferRtv_.Reset();
  backBuffer_.Reset();
  swapChain_.Reset();
  context_.Reset();
  device_.Reset();
  adapter_.Reset();
  factory_.Reset();
  windowHandle_ = nullptr;
  width_ = 0;
  height_ = 0;
}

bool D3DContext::SelectAdapter() {
  HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(factory_.ReleaseAndGetAddressOf()));
  if (FAILED(hr)) {
    return false;
  }

  for (UINT index = 0;; ++index) {
    ComPtr<IDXGIAdapter1> candidate;
    hr = factory_->EnumAdapters1(index, &candidate);
    if (hr == DXGI_ERROR_NOT_FOUND) {
      break;
    }
    if (FAILED(hr)) {
      return false;
    }

    DXGI_ADAPTER_DESC1 desc{};
    candidate->GetDesc1(&desc);
    if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0) {
      continue;
    }

    ComPtr<IDXGIOutput> output;
    if (candidate->EnumOutputs(0, &output) == S_OK) {
      adapter_ = candidate;
      return true;
    }
  }

  return false;
}

bool D3DContext::CreateDeviceAndSwapChain() {
  constexpr D3D_FEATURE_LEVEL levels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
  };

  UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
  flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
  HRESULT hr = D3D11CreateDevice(adapter_.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags, levels,
                                 static_cast<UINT>(std::size(levels)), D3D11_SDK_VERSION,
                                 &device_, &featureLevel, &context_);
  if (FAILED(hr)) {
    return false;
  }

  DXGI_SWAP_CHAIN_DESC swapChainDesc{};
  swapChainDesc.BufferDesc.Width = width_;
  swapChainDesc.BufferDesc.Height = height_;
  swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = 2;
  swapChainDesc.OutputWindow = windowHandle_;
  swapChainDesc.Windowed = TRUE;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  hr = factory_->CreateSwapChain(device_.Get(), &swapChainDesc, &swapChain_);
  if (FAILED(hr)) {
    return false;
  }

  factory_->MakeWindowAssociation(windowHandle_, DXGI_MWA_NO_ALT_ENTER);
  return true;
}

bool D3DContext::CreateBackBufferResources() {
  backBufferRtv_.Reset();
  backBuffer_.Reset();

  HRESULT hr = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer_.ReleaseAndGetAddressOf()));
  if (FAILED(hr)) {
    return false;
  }

  hr = device_->CreateRenderTargetView(backBuffer_.Get(), nullptr, &backBufferRtv_);
  if (FAILED(hr)) {
    return false;
  }

  return true;
}

bool D3DContext::Resize(UINT width, UINT height) {
  if (swapChain_ == nullptr || width == 0 || height == 0) {
    return false;
  }

  width_ = width;
  height_ = height;

  if (context_ != nullptr) {
    context_->OMSetRenderTargets(0, nullptr, nullptr);
    context_->Flush();
  }

  backBufferRtv_.Reset();
  backBuffer_.Reset();

  const HRESULT hr = swapChain_->ResizeBuffers(0, width_, height_, DXGI_FORMAT_UNKNOWN, 0);
  if (FAILED(hr)) {
    return false;
  }

  return CreateBackBufferResources();
}

void D3DContext::Clear(const float clearColor[4]) {
  if (context_ == nullptr || backBufferRtv_ == nullptr) {
    return;
  }

  context_->OMSetRenderTargets(1, backBufferRtv_.GetAddressOf(), nullptr);
  context_->ClearRenderTargetView(backBufferRtv_.Get(), clearColor);
}

bool D3DContext::Present() {
  return swapChain_ != nullptr && SUCCEEDED(swapChain_->Present(1, 0));
}

std::string D3DContext::AdapterDescription() const {
  if (adapter_ == nullptr) {
    return {};
  }

  DXGI_ADAPTER_DESC1 desc{};
  adapter_->GetDesc1(&desc);
  return supra::capture::AdapterToString(desc);
}

}  // namespace supra::renderer

#endif
