#include "capture_utils.h"

#ifdef _WIN32

#include <vector>

namespace supra::capture {

std::string WideToUtf8(const wchar_t* value) {
  if (value == nullptr || value[0] == L'\0') {
    return {};
  }

  const int size = WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
  if (size <= 1) {
    return {};
  }

  std::vector<char> buffer(static_cast<std::size_t>(size));
  WideCharToMultiByte(CP_UTF8, 0, value, -1, buffer.data(), size, nullptr, nullptr);
  return std::string(buffer.data());
}

std::string HrToString(HRESULT hr) {
  char* buffer = nullptr;
  const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
  const DWORD languageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
  const DWORD length = FormatMessageA(flags, nullptr, static_cast<DWORD>(hr), languageId,
                                      reinterpret_cast<LPSTR>(&buffer), 0, nullptr);

  std::string message;
  if (length > 0 && buffer != nullptr) {
    message.assign(buffer, buffer + length);
    while (!message.empty() && (message.back() == '\r' || message.back() == '\n')) {
      message.pop_back();
    }
  }

  if (buffer != nullptr) {
    LocalFree(buffer);
  }

  return message;
}

std::string AdapterToString(const DXGI_ADAPTER_DESC1& desc) {
  std::string result = WideToUtf8(desc.Description);
  if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
    result += " [software]";
  }
  return result;
}

std::string OutputToString(const DXGI_OUTPUT_DESC& desc) {
  return WideToUtf8(desc.DeviceName);
}

}  // namespace supra::capture

#endif
