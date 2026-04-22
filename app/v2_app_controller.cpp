#include "v2_app_controller.h"

#ifdef _WIN32

#include "window.h"

namespace supra::app {

bool V2AppController::Initialize(HWND windowHandle, UINT width, UINT height) {
  if (windowHandle == nullptr) {
    lastError_ = "Failed to initialize the V2 application shell because the window handle was null.";
    return false;
  }

  windowHandle_ = windowHandle;
  UpdateShellWindowDimensions(width, height);
  logicalWorkspaceSize_ = supra::input::LogicalWorkspaceSize{width, height};
  RebuildInputMapping();
  lastError_.clear();
  hadWindowInputFocus_ = HasWindowInputFocus();
  consumeBorderlessRestoreEscapeUntilRelease_ = false;
  initialized_ = true;

  Log("Supra View V2 Phase 1 shell controller initialized.");
  Log("Shell input is captured in window-client coordinates and remapped into logical workspace coordinates.");
  Log("The V2 shell is a direct interactive workspace surface, not a mirrored desktop window.");
  Log("Press F11 to toggle borderless fullscreen. Press Escape to restore the standard windowed mode.");
  return true;
}

void V2AppController::Shutdown() {
  inputMapper_.Reset();
  inputState_.Reset();
  logicalWorkspaceSize_ = supra::input::LogicalWorkspaceSize{};
  windowHandle_ = nullptr;
  shellWindowWidth_ = 0;
  shellWindowHeight_ = 0;
  lastError_.clear();
  hadWindowInputFocus_ = false;
  consumeBorderlessRestoreEscapeUntilRelease_ = false;
  initialized_ = false;
}

bool V2AppController::Tick() {
  if (!initialized_) {
    lastError_ = "V2 shell controller ticked before initialization.";
    return false;
  }

  const bool hasWindowInputFocus = HasWindowInputFocus();
  if (hadWindowInputFocus_ && !hasWindowInputFocus) {
    inputState_.Reset();
    consumeBorderlessRestoreEscapeUntilRelease_ = false;
  }
  hadWindowInputFocus_ = hasWindowInputFocus;

  return true;
}

void V2AppController::OnResize(UINT width, UINT height) {
  if (!initialized_ || width == 0 || height == 0) {
    return;
  }

  UpdateShellWindowDimensions(width, height);
  RebuildInputMapping();

  if (inputState_.Pointer().hasInputSurfacePosition) {
    UpdatePointerFromInputSurface(inputState_.Pointer().inputSurfacePosition);
  }
}

void V2AppController::OnMouseMove(const supra::input::MouseMoveEvent& mouseMoveEvent) {
  if (!initialized_) {
    return;
  }

  UpdatePointerFromInputSurface(mouseMoveEvent.inputSurfacePosition);
}

void V2AppController::OnMouseButton(const supra::input::MouseButtonEvent& mouseButtonEvent) {
  if (!initialized_) {
    return;
  }

  UpdatePointerFromInputSurface(mouseButtonEvent.inputSurfacePosition);
  inputState_.UpdateMouseButton(mouseButtonEvent);
}

void V2AppController::OnKeyboard(const supra::input::KeyboardEvent& keyboardEvent) {
  if (!initialized_) {
    return;
  }

  const bool handledShellShortcut = HandleShellKeyboardShortcut(keyboardEvent);
  if (handledShellShortcut) {
    return;
  }

  inputState_.UpdateKeyboardKey(keyboardEvent);
}

std::wstring V2AppController::LastErrorText() const {
  if (lastError_.empty()) {
    return L"Unknown error.";
  }

  const int length = MultiByteToWideChar(CP_UTF8, 0, lastError_.c_str(), -1, nullptr, 0);
  std::wstring result(static_cast<std::size_t>(length), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, lastError_.c_str(), -1, result.data(), length);
  if (!result.empty() && result.back() == L'\0') {
    result.pop_back();
  }
  return result;
}

void V2AppController::Log(const std::string& message) const {
  if (message.empty()) {
    return;
  }

  OutputDebugStringA(message.c_str());
  OutputDebugStringA("\n");
}

bool V2AppController::HandleShellKeyboardShortcut(
    const supra::input::KeyboardEvent& keyboardEvent) {
  Window* const window = Window::FromHandle(windowHandle_);
  if (window == nullptr) {
    return false;
  }

  switch (keyboardEvent.virtualKey) {
    case VK_F11:
      if (keyboardEvent.transition != supra::input::KeyTransition::Pressed ||
          keyboardEvent.wasPreviouslyDown) {
        return true;
      }

      if (window->ToggleBorderlessFullscreen()) {
        Log(window->IsBorderlessFullscreen()
                ? "Entered borderless fullscreen presentation mode."
                : "Restored the standard windowed presentation mode.");
      } else {
        Log("Failed to toggle the borderless fullscreen presentation mode.");
      }
      return true;
    case VK_ESCAPE:
      if (consumeBorderlessRestoreEscapeUntilRelease_) {
        if (keyboardEvent.transition == supra::input::KeyTransition::Released) {
          consumeBorderlessRestoreEscapeUntilRelease_ = false;
        }

        return true;
      }

      if (keyboardEvent.transition != supra::input::KeyTransition::Pressed ||
          keyboardEvent.wasPreviouslyDown) {
        return false;
      }

      if (!window->IsBorderlessFullscreen()) {
        return false;
      }

      if (window->SetBorderlessFullscreen(false)) {
        consumeBorderlessRestoreEscapeUntilRelease_ = true;
        Log("Restored the standard windowed presentation mode.");
      } else {
        Log("Failed to restore the standard windowed presentation mode.");
      }
      return true;
    default:
      return false;
  }
}

bool V2AppController::HasWindowInputFocus() const {
  return windowHandle_ != nullptr && GetFocus() == windowHandle_;
}

void V2AppController::RebuildInputMapping() {
  const supra::input::InputSurfaceMappingConfig config{
      supra::input::InputSurfaceSize{shellWindowWidth_, shellWindowHeight_},
      logicalWorkspaceSize_,
  };

  inputMapper_.Configure(config);
}

void V2AppController::UpdatePointerFromInputSurface(
    const supra::input::InputSurfacePoint& inputSurfacePoint) {
  inputState_.UpdatePointer(inputMapper_.MapInputSurfaceToLogicalWorkspace(inputSurfacePoint));
}

void V2AppController::UpdateShellWindowDimensions(UINT width, UINT height) {
  shellWindowWidth_ = width;
  shellWindowHeight_ = height;
}

}  // namespace supra::app

#endif
