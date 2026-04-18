#pragma once

#include <string>

#ifdef _WIN32

#include <windows.h>
#include <dxgi.h>
#include <dxgi1_2.h>

namespace supra::capture {

std::string HrToString(HRESULT hr);
std::string WideToUtf8(const wchar_t* value);
std::string AdapterToString(const DXGI_ADAPTER_DESC1& desc);
std::string OutputToString(const DXGI_OUTPUT_DESC& desc);

}  // namespace supra::capture

#else

namespace supra::capture {

inline std::string HrToString(long) { return {}; }
inline std::string WideToUtf8(const wchar_t*) { return {}; }
inline std::string AdapterToString(...) { return {}; }
inline std::string OutputToString(...) { return {}; }

}  // namespace supra::capture

#endif
