#include "input_state.h"

namespace supra {
namespace input {

namespace {

void SetMouseButtonPressedState(MouseButtonState& mouseButtonState,
                                MouseButton button,
                                bool isPressed) {
  switch (button) {
    case MouseButton::Left:
      mouseButtonState.left = isPressed;
      break;
    case MouseButton::Right:
      mouseButtonState.right = isPressed;
      break;
    case MouseButton::Middle:
      mouseButtonState.middle = isPressed;
      break;
    case MouseButton::X1:
      mouseButtonState.x1 = isPressed;
      break;
    case MouseButton::X2:
      mouseButtonState.x2 = isPressed;
      break;
  }
}

}  // namespace

void InputState::Reset() {
  pointer_ = {};
  mouseButtons_ = {};
  keyStates_.fill(false);
}

void InputState::UpdatePointer(const MappedWorkspacePoint& mappedWorkspacePoint) {
  pointer_.inputSurfacePosition = mappedWorkspacePoint.inputSurfacePosition;
  pointer_.logicalWorkspacePosition = mappedWorkspacePoint.logicalWorkspacePosition;
  pointer_.hasInputSurfacePosition = true;
  pointer_.isWithinInputSurface = mappedWorkspacePoint.isWithinInputSurface;
  pointer_.isMappedToLogicalWorkspace = mappedWorkspacePoint.isMappedToLogicalWorkspace;
}

void InputState::UpdateMouseButton(const MouseButtonEvent& mouseButtonEvent) {
  SetMouseButtonPressedState(mouseButtons_, mouseButtonEvent.button,
                             mouseButtonEvent.transition == ButtonTransition::Pressed);
}

void InputState::UpdateKeyboardKey(const KeyboardEvent& keyboardEvent) {
  if (keyboardEvent.virtualKey >= keyStates_.size()) {
    return;
  }

  keyStates_[keyboardEvent.virtualKey] = keyboardEvent.transition == KeyTransition::Pressed;
}

bool InputState::IsMouseButtonPressed(MouseButton button) const noexcept {
  switch (button) {
    case MouseButton::Left:
      return mouseButtons_.left;
    case MouseButton::Right:
      return mouseButtons_.right;
    case MouseButton::Middle:
      return mouseButtons_.middle;
    case MouseButton::X1:
      return mouseButtons_.x1;
    case MouseButton::X2:
      return mouseButtons_.x2;
  }

  return false;
}

bool InputState::IsKeyPressed(std::uint32_t virtualKey) const noexcept {
  if (virtualKey >= keyStates_.size()) {
    return false;
  }

  return keyStates_[virtualKey];
}

}  // namespace input
}  // namespace supra
