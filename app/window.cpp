#include "window.h"

#ifdef _WIN32

namespace supra::app {

namespace {

constexpr wchar_t kWindowClassName[] = L"SupraViewWindowClass";

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
    case WM_SIZE: {
      clientWidth_ = LOWORD(lParam);
      clientHeight_ = HIWORD(lParam);
      if (resizeCallback_ != nullptr && wParam != SIZE_MINIMIZED) {
        resizeCallback_(clientWidth_, clientHeight_);
      }
      return 0;
    }
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    default:
      break;
  }

  return DefWindowProcW(windowHandle_, message, wParam, lParam);
}

}  // namespace supra::app

#endif
