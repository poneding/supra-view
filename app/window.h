#pragma once

#include "../core/input/input_state.h"

#include <cstdint>
#include <functional>
#include <string_view>
#include <utility>

#ifdef _WIN32

#include <windows.h>

namespace supra {
namespace app {

class Window {
 public:
  using ResizeCallback = std::function<void(std::uint32_t, std::uint32_t)>;
  using MouseMoveCallback = std::function<void(const supra::input::MouseMoveEvent&)>;
  using MouseButtonCallback = std::function<void(const supra::input::MouseButtonEvent&)>;
  using KeyboardCallback = std::function<void(const supra::input::KeyboardEvent&)>;

  bool Create(std::wstring_view title, int clientWidth, int clientHeight);
  void Destroy();
  bool PumpMessages();
  bool SetBorderlessFullscreen(bool enabled);
  bool ToggleBorderlessFullscreen();

  void SetResizeCallback(ResizeCallback callback) { resizeCallback_ = std::move(callback); }
  void SetMouseMoveCallback(MouseMoveCallback callback) { mouseMoveCallback_ = std::move(callback); }
  void SetMouseButtonCallback(MouseButtonCallback callback) {
    mouseButtonCallback_ = std::move(callback);
  }
  void SetKeyboardCallback(KeyboardCallback callback) { keyboardCallback_ = std::move(callback); }

  static Window* FromHandle(HWND windowHandle);

  [[nodiscard]] HWND Handle() const noexcept { return windowHandle_; }
  [[nodiscard]] UINT ClientWidth() const noexcept { return clientWidth_; }
  [[nodiscard]] UINT ClientHeight() const noexcept { return clientHeight_; }
  [[nodiscard]] bool IsBorderlessFullscreen() const noexcept { return borderlessFullscreen_; }

 private:
  bool EnterBorderlessFullscreen();
  bool ExitBorderlessFullscreen();

  static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  HWND windowHandle_ = nullptr;
  UINT clientWidth_ = 0;
  UINT clientHeight_ = 0;
  bool hasInputFocus_ = false;
  bool borderlessFullscreen_ = false;
  LONG_PTR windowedStyle_ = 0;
  LONG_PTR windowedExStyle_ = 0;
  WINDOWPLACEMENT windowedPlacement_{sizeof(WINDOWPLACEMENT)};
  ResizeCallback resizeCallback_;
  MouseMoveCallback mouseMoveCallback_;
  MouseButtonCallback mouseButtonCallback_;
  KeyboardCallback keyboardCallback_;
};

}  // namespace app
}  // namespace supra

#else

namespace supra {
namespace app {

class Window {
 public:
  using ResizeCallback = std::function<void(std::uint32_t, std::uint32_t)>;
  using MouseMoveCallback = std::function<void(const supra::input::MouseMoveEvent&)>;
  using MouseButtonCallback = std::function<void(const supra::input::MouseButtonEvent&)>;
  using KeyboardCallback = std::function<void(const supra::input::KeyboardEvent&)>;

  bool Create(std::wstring_view, int clientWidth, int clientHeight) {
    clientWidth_ = static_cast<std::uint32_t>(clientWidth);
    clientHeight_ = static_cast<std::uint32_t>(clientHeight);
    return false;
  }
  void Destroy() {}
  bool PumpMessages() { return false; }
  bool SetBorderlessFullscreen(bool) { return false; }
  bool ToggleBorderlessFullscreen() { return false; }
  void SetResizeCallback(ResizeCallback callback) { resizeCallback_ = std::move(callback); }
  void SetMouseMoveCallback(MouseMoveCallback callback) { mouseMoveCallback_ = std::move(callback); }
  void SetMouseButtonCallback(MouseButtonCallback callback) {
    mouseButtonCallback_ = std::move(callback);
  }
  void SetKeyboardCallback(KeyboardCallback callback) { keyboardCallback_ = std::move(callback); }
  static Window* FromHandle(void*) { return nullptr; }
  [[nodiscard]] void* Handle() const noexcept { return nullptr; }
  [[nodiscard]] std::uint32_t ClientWidth() const noexcept { return clientWidth_; }
  [[nodiscard]] std::uint32_t ClientHeight() const noexcept { return clientHeight_; }
  [[nodiscard]] bool IsBorderlessFullscreen() const noexcept { return false; }

 private:
  std::uint32_t clientWidth_ = 0;
  std::uint32_t clientHeight_ = 0;
  ResizeCallback resizeCallback_;
  MouseMoveCallback mouseMoveCallback_;
  MouseButtonCallback mouseButtonCallback_;
  KeyboardCallback keyboardCallback_;
};

}  // namespace app
}  // namespace supra

#endif
