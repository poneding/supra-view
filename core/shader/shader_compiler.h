#pragma once

#include <string>

#ifdef _WIN32

#include <windows.h>
#include <d3d11.h>

namespace supra::shader {

struct CompileResult {
  bool succeeded = false;
  HRESULT hr = E_FAIL;
  std::string message;
};

CompileResult CompileVertexShader(ID3D11Device* device, const char* source, const char* entryPoint,
                                  ID3D11VertexShader** shaderOut);
CompileResult CompilePixelShader(ID3D11Device* device, const char* source, const char* entryPoint,
                                 ID3D11PixelShader** shaderOut);

}  // namespace supra::shader

#else

namespace supra::shader {

struct CompileResult {
  bool succeeded = false;
  long hr = -1;
  std::string message;
};

CompileResult CompileVertexShader(void*, const char*, const char*, void**);
CompileResult CompilePixelShader(void*, const char*, const char*, void**);

}  // namespace supra::shader

#endif
