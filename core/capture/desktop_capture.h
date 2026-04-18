#pragma once

#include "capture_types.h"

#include <string>

#ifdef _WIN32

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

namespace supra::capture {

class DesktopCapture {
 public:
  bool Initialize(ID3D11Device* device, const CaptureConfig& config);
  void Shutdown();

  [[nodiscard]] CaptureResult AcquireFrame();
  bool ReleaseFrame();
  bool Reinitialize();

  [[nodiscard]] std::string CurrentOutputName() const;

 private:
  bool InitializeDuplication();

  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<IDXGIOutput1> output1_;
  Microsoft::WRL::ComPtr<IDXGIOutputDuplication> duplication_;
  DXGI_OUTDUPL_DESC duplicationDesc_{};
  CaptureConfig config_{};
  bool frameAcquired_ = false;
  std::wstring outputName_;
};

}  // namespace supra::capture

#else

namespace supra::capture {

class DesktopCapture {
 public:
  bool Initialize(void*, const CaptureConfig&) { return false; }
  void Shutdown() {}
  [[nodiscard]] CaptureResult AcquireFrame() { return {}; }
  bool ReleaseFrame() { return true; }
  bool Reinitialize() { return false; }
  [[nodiscard]] std::string CurrentOutputName() const { return {}; }
};

}  // namespace supra::capture

#endif
