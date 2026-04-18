#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <utility>

#ifdef _WIN32

#include <windows.h>

namespace supra::app {

class Window {
 public:
  using ResizeCallback = std::function<void(std::uint32_t, std::uint32_t)>;

  bool Create(std::wstring_view title, int clientWidth, int clientHeight);
  void Destroy();
  bool PumpMessages();

  void SetResizeCallback(ResizeCallback callback) { resizeCallback_ = std::move(callback); }

  [[nodiscard]] HWND Handle() const noexcept { return windowHandle_; }
  [[nodiscard]] UINT ClientWidth() const noexcept { return clientWidth_; }
  [[nodiscard]] UINT ClientHeight() const noexcept { return clientHeight_; }

 private:
  static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  HWND windowHandle_ = nullptr;
  UINT clientWidth_ = 0;
  UINT clientHeight_ = 0;
  ResizeCallback resizeCallback_;
};

}  // namespace supra::app

#else

namespace supra::app {

class Window {
 public:
  using ResizeCallback = std::function<void(std::uint32_t, std::uint32_t)>;

  bool Create(std::wstring_view, int clientWidth, int clientHeight) {
    clientWidth_ = static_cast<std::uint32_t>(clientWidth);
    clientHeight_ = static_cast<std::uint32_t>(clientHeight);
    return false;
  }
  void Destroy() {}
  bool PumpMessages() { return false; }
  void SetResizeCallback(ResizeCallback callback) { resizeCallback_ = std::move(callback); }
  [[nodiscard]] void* Handle() const noexcept { return nullptr; }
  [[nodiscard]] std::uint32_t ClientWidth() const noexcept { return clientWidth_; }
  [[nodiscard]] std::uint32_t ClientHeight() const noexcept { return clientHeight_; }

 private:
  std::uint32_t clientWidth_ = 0;
  std::uint32_t clientHeight_ = 0;
  ResizeCallback resizeCallback_;
};

}  // namespace supra::app

#endif
