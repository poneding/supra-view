#include "app_controller.h"
#include "window.h"

#ifdef _WIN32

#include <windows.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
  if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
    MessageBoxW(nullptr, L"Failed to initialize COM.", L"Supra View", MB_ICONERROR | MB_OK);
    return -1;
  }

  constexpr int kInitialWidth = 2560;
  constexpr int kInitialHeight = 1440;

  supra::app::Window window;
  if (!window.Create(L"Supra View", kInitialWidth, kInitialHeight)) {
    CoUninitialize();
    return -1;
  }

  supra::app::AppController controller;
  window.SetResizeCallback([&controller](std::uint32_t width, std::uint32_t height) {
    controller.OnResize(width, height);
  });

  if (!controller.Initialize(window.Handle(), window.ClientWidth(), window.ClientHeight())) {
    MessageBoxW(window.Handle(), controller.LastErrorText().c_str(), L"Supra View", MB_ICONERROR | MB_OK);
    window.Destroy();
    CoUninitialize();
    return -1;
  }

  bool running = true;
  while (running) {
    running = window.PumpMessages();
    if (!running) {
      break;
    }

    if (!controller.Tick()) {
      MessageBoxW(window.Handle(), controller.LastErrorText().c_str(), L"Supra View", MB_ICONERROR | MB_OK);
      break;
    }
  }

  controller.Shutdown();
  window.Destroy();
  CoUninitialize();
  return 0;
}

#endif
