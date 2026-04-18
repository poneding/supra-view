#pragma once

#include <string>

#ifdef _WIN32

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

namespace supra::capture {

enum class CaptureStatus {
  FrameAvailable,
  Timeout,
  AccessLost,
  Error,
};

struct CaptureConfig {
  UINT outputIndex = 0;
  UINT timeoutMilliseconds = 8;
};

struct CapturedFrame {
  Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
  UINT width = 0;
  UINT height = 0;
  DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
  LARGE_INTEGER lastPresentTime{};
  UINT accumulatedFrames = 0;

  [[nodiscard]] bool IsValid() const noexcept {
    return texture != nullptr;
  }
};

struct CaptureResult {
  CaptureStatus status = CaptureStatus::Error;
  CapturedFrame frame{};
  HRESULT hr = E_FAIL;
  std::string message;
};

}  // namespace supra::capture

#else

#include <cstdint>

namespace supra::capture {

enum class CaptureStatus {
  FrameAvailable,
  Timeout,
  AccessLost,
  Error,
};

struct CaptureConfig {
  std::uint32_t outputIndex = 0;
  std::uint32_t timeoutMilliseconds = 8;
};

struct CapturedFrame {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t format = 0;
  long long lastPresentTime = 0;
  std::uint32_t accumulatedFrames = 0;

  [[nodiscard]] bool IsValid() const noexcept {
    return false;
  }
};

struct CaptureResult {
  CaptureStatus status = CaptureStatus::Error;
  CapturedFrame frame{};
  long hr = -1;
  std::string message;
};

}  // namespace supra::capture

#endif
