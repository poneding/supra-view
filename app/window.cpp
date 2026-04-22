#include "window.h"

#ifdef _WIN32

namespace supra::app {

namespace {

constexpr wchar_t kWindowClassName[] = L"SupraViewWindowClass";

bool UpdateWindowLongPtr(HWND windowHandle, int index, LONG_PTR value) {
  SetLastError(ERROR_SUCCESS);
  const LONG_PTR result = SetWindowLongPtrW(windowHandle, index, value);
  return result != 0 || GetLastError() == ERROR_SUCCESS;
}

supra::input::InputSurfacePoint MakeInputSurfacePointFromWindowClient(LPARAM lParam) {
  supra::input::InputSurfacePoint point;
  point.x = GET_X_LPARAM(lParam);
  point.y = GET_Y_LPARAM(lParam);
  return point;
}

supra::input::MouseButton ResolveMouseButton(UINT message, WPARAM wParam) {
  switch (message) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      return supra::input::MouseButton::Left;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      return supra::input::MouseButton::Right;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      return supra::input::MouseButton::Middle;
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
      return GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? supra::input::MouseButton::X1
                                                    : supra::input::MouseButton::X2;
    default:
      return supra::input::MouseButton::Left;
  }
}

supra::input::ButtonTransition ResolveButtonTransition(UINT message) {
  switch (message) {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
      return supra::input::ButtonTransition::Pressed;
    default:
      return supra::input::ButtonTransition::Released;
  }
}

supra::input::KeyTransition ResolveKeyTransition(UINT message) {
  switch (message) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
      return supra::input::KeyTransition::Pressed;
    default:
      return supra::input::KeyTransition::Released;
  }
}

}  // namespace

bool Window::Create(std::wstring_view title, int clientWidth, int clientHeight) {
  Destroy();

  HINSTANCE instance = GetModuleHandleW(nullptr);

  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.hInstance = instance;
  windowClass.lpfnWndProc = &Window::StaticWindowProc;
  windowClass.lpszClassName = kWindowClassName;
  windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  windowClass.style = CS_HREDRAW | CS_VREDRAW;

  RegisterClassExW(&windowClass);

  RECT windowRect{0, 0, clientWidth, clientHeight};
  AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

  windowHandle_ = CreateWindowExW(0, kWindowClassName, title.data(),
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  windowRect.right - windowRect.left,
                                  windowRect.bottom - windowRect.top,
                                  nullptr, nullptr, instance, this);

  if (windowHandle_ == nullptr) {
    return false;
  }

  clientWidth_ = static_cast<UINT>(clientWidth);
  clientHeight_ = static_cast<UINT>(clientHeight);
  ShowWindow(windowHandle_, SW_SHOW);
  UpdateWindow(windowHandle_);
  return true;
}

void Window::Destroy() {
  if (windowHandle_ != nullptr) {
    DestroyWindow(windowHandle_);
    windowHandle_ = nullptr;
  }

  hasInputFocus_ = false;
  borderlessFullscreen_ = false;
  windowedStyle_ = 0;
  windowedExStyle_ = 0;
  windowedPlacement_ = WINDOWPLACEMENT{sizeof(WINDOWPLACEMENT)};
}

bool Window::PumpMessages() {
  MSG message{};
  while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
    if (message.message == WM_QUIT) {
      return false;
    }

    TranslateMessage(&message);
    DispatchMessageW(&message);
  }

  return true;
}

bool Window::SetBorderlessFullscreen(bool enabled) {
  if (windowHandle_ == nullptr) {
    return false;
  }

  if (enabled == borderlessFullscreen_) {
    return true;
  }

  return enabled ? EnterBorderlessFullscreen() : ExitBorderlessFullscreen();
}

bool Window::ToggleBorderlessFullscreen() {
  return SetBorderlessFullscreen(!borderlessFullscreen_);
}

Window* Window::FromHandle(HWND windowHandle) {
  if (windowHandle == nullptr) {
    return nullptr;
  }

  return reinterpret_cast<Window*>(GetWindowLongPtrW(windowHandle, GWLP_USERDATA));
}

bool Window::EnterBorderlessFullscreen() {
  MONITORINFO monitorInfo{};
  monitorInfo.cbSize = sizeof(monitorInfo);
  const HMONITOR monitorHandle = MonitorFromWindow(windowHandle_, MONITOR_DEFAULTTONEAREST);
  if (monitorHandle == nullptr || !GetMonitorInfoW(monitorHandle, &monitorInfo)) {
    return false;
  }

  windowedPlacement_.length = sizeof(windowedPlacement_);
  if (!GetWindowPlacement(windowHandle_, &windowedPlacement_)) {
    return false;
  }

  windowedStyle_ = GetWindowLongPtrW(windowHandle_, GWL_STYLE);
  windowedExStyle_ = GetWindowLongPtrW(windowHandle_, GWL_EXSTYLE);

  RECT windowedRect{};
  const bool hasWindowedRect = GetWindowRect(windowHandle_, &windowedRect) != FALSE;

  const auto restoreWindowedShellState = [&]() {
    bool restored = UpdateWindowLongPtr(windowHandle_, GWL_STYLE, windowedStyle_);
    restored = UpdateWindowLongPtr(windowHandle_, GWL_EXSTYLE, windowedExStyle_) && restored;

    restored = SetWindowPlacement(windowHandle_, &windowedPlacement_) && restored;

    if (hasWindowedRect) {
      restored = SetWindowPos(windowHandle_, nullptr, windowedRect.left, windowedRect.top,
                              windowedRect.right - windowedRect.left,
                              windowedRect.bottom - windowedRect.top,
                              SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOZORDER) &&
                 restored;
    }

    return restored;
  };

  const LONG_PTR fullscreenStyle = windowedStyle_ & ~static_cast<LONG_PTR>(WS_OVERLAPPEDWINDOW);
  const LONG_PTR fullscreenExStyle = windowedExStyle_ & ~static_cast<LONG_PTR>(WS_EX_OVERLAPPEDWINDOW);
  const bool updatedStyle = UpdateWindowLongPtr(windowHandle_, GWL_STYLE, fullscreenStyle);
  const bool updatedExStyle = UpdateWindowLongPtr(windowHandle_, GWL_EXSTYLE, fullscreenExStyle);
  if (!updatedStyle || !updatedExStyle) {
    restoreWindowedShellState();
    return false;
  }

  const RECT monitorRect = monitorInfo.rcMonitor;
  if (!SetWindowPos(windowHandle_, HWND_TOP, monitorRect.left, monitorRect.top,
                    monitorRect.right - monitorRect.left, monitorRect.bottom - monitorRect.top,
                    SWP_FRAMECHANGED | SWP_NOOWNERZORDER)) {
    restoreWindowedShellState();
    return false;
  }

  borderlessFullscreen_ = true;
  return true;
}

bool Window::ExitBorderlessFullscreen() {
  const LONG_PTR fullscreenStyle = GetWindowLongPtrW(windowHandle_, GWL_STYLE);
  const LONG_PTR fullscreenExStyle = GetWindowLongPtrW(windowHandle_, GWL_EXSTYLE);

  RECT fullscreenRect{};
  const bool hasFullscreenRect = GetWindowRect(windowHandle_, &fullscreenRect) != FALSE;

  WINDOWPLACEMENT fullscreenPlacement{sizeof(WINDOWPLACEMENT)};
  const bool hasFullscreenPlacement =
      GetWindowPlacement(windowHandle_, &fullscreenPlacement) != FALSE;

  const auto restoreFullscreenShellState = [&]() {
    bool restored = UpdateWindowLongPtr(windowHandle_, GWL_STYLE, fullscreenStyle);
    restored = UpdateWindowLongPtr(windowHandle_, GWL_EXSTYLE, fullscreenExStyle) && restored;

    if (hasFullscreenPlacement) {
      restored = SetWindowPlacement(windowHandle_, &fullscreenPlacement) && restored;
    }

    if (hasFullscreenRect) {
      restored = SetWindowPos(windowHandle_, nullptr, fullscreenRect.left, fullscreenRect.top,
                              fullscreenRect.right - fullscreenRect.left,
                              fullscreenRect.bottom - fullscreenRect.top,
                              SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOZORDER) &&
                 restored;
    }

    return restored;
  };

  const bool restoredStyle = UpdateWindowLongPtr(windowHandle_, GWL_STYLE, windowedStyle_);
  const bool restoredExStyle = UpdateWindowLongPtr(windowHandle_, GWL_EXSTYLE, windowedExStyle_);
  if (!restoredStyle || !restoredExStyle) {
    restoreFullscreenShellState();
    return false;
  }

  windowedPlacement_.length = sizeof(windowedPlacement_);
  if (!SetWindowPlacement(windowHandle_, &windowedPlacement_)) {
    restoreFullscreenShellState();
    return false;
  }

  if (!SetWindowPos(windowHandle_, nullptr, 0, 0, 0, 0,
                    SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER |
                        SWP_NOZORDER)) {
    restoreFullscreenShellState();
    return false;
  }

  borderlessFullscreen_ = false;
  return true;
}

LRESULT CALLBACK Window::StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  Window* window = nullptr;
  if (message == WM_NCCREATE) {
    auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
    window = static_cast<Window*>(createStruct->lpCreateParams);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    window->windowHandle_ = hwnd;
  } else {
    window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  }

  if (window != nullptr) {
    return window->WindowProc(message, wParam, lParam);
  }

  return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT Window::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_SETFOCUS:
      hasInputFocus_ = true;
      return 0;
    case WM_KILLFOCUS:
      hasInputFocus_ = false;
      return 0;
    case WM_SIZE: {
      clientWidth_ = LOWORD(lParam);
      clientHeight_ = HIWORD(lParam);
      if (resizeCallback_ != nullptr && wParam != SIZE_MINIMIZED) {
        resizeCallback_(clientWidth_, clientHeight_);
      }
      return 0;
    }
    case WM_MOUSEMOVE: {
      if (hasInputFocus_ && mouseMoveCallback_ != nullptr) {
        mouseMoveCallback_(
            supra::input::MouseMoveEvent{MakeInputSurfacePointFromWindowClient(lParam)});
      }
      return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP: {
      if (hasInputFocus_ && mouseButtonCallback_ != nullptr) {
        supra::input::MouseButtonEvent mouseButtonEvent;
        mouseButtonEvent.button = ResolveMouseButton(message, wParam);
        mouseButtonEvent.transition = ResolveButtonTransition(message);
        mouseButtonEvent.inputSurfacePosition = MakeInputSurfacePointFromWindowClient(lParam);
        mouseButtonCallback_(mouseButtonEvent);
      }

      return message == WM_XBUTTONDOWN || message == WM_XBUTTONUP ? TRUE : 0;
    }
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP: {
      if (hasInputFocus_ && keyboardCallback_ != nullptr) {
        supra::input::KeyboardEvent keyboardEvent;
        keyboardEvent.virtualKey = static_cast<std::uint32_t>(wParam);
        keyboardEvent.transition = ResolveKeyTransition(message);
        keyboardEvent.wasPreviouslyDown = (lParam & (1UL << 30)) != 0;
        keyboardEvent.isExtendedKey = (lParam & (1UL << 24)) != 0;
        keyboardEvent.repeatCount = static_cast<std::uint16_t>(lParam & 0xFFFFUL);
        keyboardCallback_(keyboardEvent);
      }

      break;
    }
    case WM_DESTROY:
      hasInputFocus_ = false;
      PostQuitMessage(0);
      return 0;
    default:
      break;
  }

  return DefWindowProcW(windowHandle_, message, wParam, lParam);
}

}  // namespace supra::app

#endif
