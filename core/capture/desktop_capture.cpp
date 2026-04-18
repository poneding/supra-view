#include "desktop_capture.h"

#ifdef _WIN32

#include "capture_utils.h"

#include <dxgi1_2.h>
#include <wrl/client.h>

namespace supra::capture {

using Microsoft::WRL::ComPtr;

bool DesktopCapture::Initialize(ID3D11Device* device, const CaptureConfig& config) {
  Shutdown();

  if (device == nullptr) {
    return false;
  }

  device_ = device;
  config_ = config;
  return InitializeDuplication();
}

bool DesktopCapture::InitializeDuplication() {
  ComPtr<IDXGIDevice> dxgiDevice;
  HRESULT hr = device_.As(&dxgiDevice);
  if (FAILED(hr)) {
    return false;
  }

  ComPtr<IDXGIAdapter> adapter;
  hr = dxgiDevice->GetAdapter(&adapter);
  if (FAILED(hr)) {
    return false;
  }

  ComPtr<IDXGIOutput> output;
  hr = adapter->EnumOutputs(config_.outputIndex, &output);
  if (FAILED(hr)) {
    return false;
  }

  DXGI_OUTPUT_DESC outputDesc{};
  if (SUCCEEDED(output->GetDesc(&outputDesc))) {
    outputName_ = outputDesc.DeviceName;
  }

  hr = output.As(&output1_);
  if (FAILED(hr)) {
    return false;
  }

  hr = output1_->DuplicateOutput(device_.Get(), &duplication_);
  if (FAILED(hr)) {
    output1_.Reset();
    return false;
  }

  duplication_->GetDesc(&duplicationDesc_);
  frameAcquired_ = false;
  return true;
}

void DesktopCapture::Shutdown() {
  ReleaseFrame();
  duplication_.Reset();
  output1_.Reset();
  device_.Reset();
  outputName_.clear();
}

CaptureResult DesktopCapture::AcquireFrame() {
  CaptureResult result{};
  result.hr = E_FAIL;
  result.status = CaptureStatus::Error;

  if (duplication_ == nullptr) {
    result.message = "Desktop duplication is not initialized.";
    return result;
  }

  if (frameAcquired_) {
    ReleaseFrame();
  }

  DXGI_OUTDUPL_FRAME_INFO frameInfo{};
  ComPtr<IDXGIResource> resource;
  const HRESULT hr = duplication_->AcquireNextFrame(config_.timeoutMilliseconds, &frameInfo, &resource);

  if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
    result.hr = hr;
    result.status = CaptureStatus::Timeout;
    result.message = "No new frame was available before the timeout expired.";
    return result;
  }

  if (hr == DXGI_ERROR_ACCESS_LOST) {
    result.hr = hr;
    result.status = CaptureStatus::AccessLost;
    result.message = "Desktop duplication access was lost and must be recreated.";
    return result;
  }

  if (FAILED(hr)) {
    result.hr = hr;
    result.status = CaptureStatus::Error;
    result.message = HrToString(hr);
    return result;
  }

  ComPtr<ID3D11Texture2D> texture;
  result.hr = resource.As(&texture);
  if (FAILED(result.hr)) {
    duplication_->ReleaseFrame();
    result.status = CaptureStatus::Error;
    result.message = "Failed to query the acquired frame as ID3D11Texture2D.";
    return result;
  }

  D3D11_TEXTURE2D_DESC textureDesc{};
  texture->GetDesc(&textureDesc);

  frameAcquired_ = true;
  result.status = CaptureStatus::FrameAvailable;
  result.hr = S_OK;
  result.frame.texture = texture;
  result.frame.width = textureDesc.Width;
  result.frame.height = textureDesc.Height;
  result.frame.format = textureDesc.Format;
  result.frame.lastPresentTime = frameInfo.LastPresentTime;
  result.frame.accumulatedFrames = frameInfo.AccumulatedFrames;
  result.message = "Frame acquired successfully.";
  return result;
}

bool DesktopCapture::ReleaseFrame() {
  if (!frameAcquired_ || duplication_ == nullptr) {
    return true;
  }

  const HRESULT hr = duplication_->ReleaseFrame();
  frameAcquired_ = false;
  return SUCCEEDED(hr);
}

bool DesktopCapture::Reinitialize() {
  if (device_ == nullptr) {
    return false;
  }

  ReleaseFrame();
  duplication_.Reset();
  output1_.Reset();
  return InitializeDuplication();
}

std::string DesktopCapture::CurrentOutputName() const {
  return WideToUtf8(outputName_.c_str());
}

}  // namespace supra::capture

#endif
