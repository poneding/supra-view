#include "shader_compiler.h"

#ifdef _WIN32

#include <d3dcompiler.h>
#include <cstring>
#include <wrl/client.h>

namespace supra::shader {

using Microsoft::WRL::ComPtr;

namespace {

CompileResult CompileShaderBlob(const char* source, const char* entryPoint, const char* profile, ID3DBlob** blobOut) {
  CompileResult result{};

  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ComPtr<ID3DBlob> shaderBlob;
  ComPtr<ID3DBlob> errorBlob;

  result.hr = D3DCompile(source, strlen(source), nullptr, nullptr, nullptr, entryPoint, profile, flags, 0,
                         &shaderBlob, &errorBlob);
  if (FAILED(result.hr)) {
    if (errorBlob != nullptr) {
      result.message.assign(static_cast<const char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
    }
    return result;
  }

  *blobOut = shaderBlob.Detach();
  result.succeeded = true;
  result.message = "Shader compiled successfully.";
  return result;
}

}  // namespace

CompileResult CompileVertexShader(ID3D11Device* device, const char* source, const char* entryPoint,
                                  ID3D11VertexShader** shaderOut) {
  if (device == nullptr || source == nullptr || entryPoint == nullptr || shaderOut == nullptr) {
    return {};
  }

  ComPtr<ID3DBlob> blob;
  CompileResult result = CompileShaderBlob(source, entryPoint, "vs_5_0", &blob);
  if (!result.succeeded) {
    return result;
  }

  result.hr = device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shaderOut);
  result.succeeded = SUCCEEDED(result.hr);
  if (!result.succeeded) {
    result.message = "Failed to create vertex shader.";
  }
  return result;
}

CompileResult CompilePixelShader(ID3D11Device* device, const char* source, const char* entryPoint,
                                 ID3D11PixelShader** shaderOut) {
  if (device == nullptr || source == nullptr || entryPoint == nullptr || shaderOut == nullptr) {
    return {};
  }

  ComPtr<ID3DBlob> blob;
  CompileResult result = CompileShaderBlob(source, entryPoint, "ps_5_0", &blob);
  if (!result.succeeded) {
    return result;
  }

  result.hr = device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shaderOut);
  result.succeeded = SUCCEEDED(result.hr);
  if (!result.succeeded) {
    result.message = "Failed to create pixel shader.";
  }
  return result;
}

}  // namespace supra::shader

#else

namespace supra::shader {

CompileResult CompileVertexShader(void*, const char*, const char*, void**) {
  return {};
}

CompileResult CompilePixelShader(void*, const char*, const char*, void**) {
  return {};
}

}  // namespace supra::shader

#endif
