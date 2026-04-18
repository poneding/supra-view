#pragma once

#include <string>

#ifdef _WIN32

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

namespace supra::renderer {

class D3DContext {
 public:
  bool Initialize(HWND windowHandle, UINT width, UINT height);
  void Shutdown();

  bool Resize(UINT width, UINT height);
  void Clear(const float clearColor[4]);
  bool Present();

  [[nodiscard]] ID3D11Device* Device() const noexcept { return device_.Get(); }
  [[nodiscard]] ID3D11DeviceContext* Context() const noexcept { return context_.Get(); }
  [[nodiscard]] ID3D11RenderTargetView* BackBufferRtv() const noexcept { return backBufferRtv_.Get(); }
  [[nodiscard]] UINT Width() const noexcept { return width_; }
  [[nodiscard]] UINT Height() const noexcept { return height_; }
  [[nodiscard]] std::string AdapterDescription() const;

 private:
  bool CreateDeviceAndSwapChain();
  bool CreateBackBufferResources();
  bool SelectAdapter();

  HWND windowHandle_ = nullptr;
  UINT width_ = 0;
  UINT height_ = 0;

  Microsoft::WRL::ComPtr<IDXGIFactory1> factory_;
  Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter_;
  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
  Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer_;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRtv_;
};

}  // namespace supra::renderer

#else

namespace supra::renderer {

class D3DContext {
 public:
  bool Initialize(void*, unsigned int width, unsigned int height) {
    width_ = width;
    height_ = height;
    return false;
  }
  void Shutdown() { width_ = 0; height_ = 0; }
  bool Resize(unsigned int width, unsigned int height) { width_ = width; height_ = height; return false; }
  void Clear(const float[4]) {}
  bool Present() { return false; }
  [[nodiscard]] void* Device() const noexcept { return nullptr; }
  [[nodiscard]] void* Context() const noexcept { return nullptr; }
  [[nodiscard]] void* BackBufferRtv() const noexcept { return nullptr; }
  [[nodiscard]] unsigned int Width() const noexcept { return width_; }
  [[nodiscard]] unsigned int Height() const noexcept { return height_; }
  [[nodiscard]] std::string AdapterDescription() const { return {}; }

 private:
  unsigned int width_ = 0;
  unsigned int height_ = 0;
};

}  // namespace supra::renderer

#endif
