#pragma once

#include <array>
#include <cstdint>

namespace supra {
namespace input {

struct InputSurfacePoint {
  std::int32_t x = 0;
  std::int32_t y = 0;
};

struct InputSurfaceSize {
  std::uint32_t width = 0;
  std::uint32_t height = 0;

  [[nodiscard]] bool IsEmpty() const noexcept { return width == 0 || height == 0; }
};

struct LogicalWorkspacePoint {
  float x = 0.0f;
  float y = 0.0f;
};

struct LogicalWorkspaceSize {
  std::uint32_t width = 0;
  std::uint32_t height = 0;

  [[nodiscard]] bool IsEmpty() const noexcept { return width == 0 || height == 0; }
};

enum class MouseButton : std::uint8_t {
  Left = 0,
  Right = 1,
  Middle = 2,
  X1 = 3,
  X2 = 4,
};

enum class ButtonTransition : std::uint8_t {
  Released = 0,
  Pressed = 1,
};

enum class KeyTransition : std::uint8_t {
  Released = 0,
  Pressed = 1,
};

struct MouseMoveEvent {
  InputSurfacePoint inputSurfacePosition{};
};

struct MouseButtonEvent {
  MouseButton button = MouseButton::Left;
  ButtonTransition transition = ButtonTransition::Released;
  InputSurfacePoint inputSurfacePosition{};
};

struct KeyboardEvent {
  std::uint32_t virtualKey = 0;
  KeyTransition transition = KeyTransition::Released;
  bool wasPreviouslyDown = false;
  bool isExtendedKey = false;
  std::uint16_t repeatCount = 0;
};

struct MappedWorkspacePoint {
  InputSurfacePoint inputSurfacePosition{};
  LogicalWorkspacePoint logicalWorkspacePosition{};
  bool isWithinInputSurface = false;
  bool isMappedToLogicalWorkspace = false;
};

struct PointerState {
  InputSurfacePoint inputSurfacePosition{};
  LogicalWorkspacePoint logicalWorkspacePosition{};
  bool hasInputSurfacePosition = false;
  bool isWithinInputSurface = false;
  bool isMappedToLogicalWorkspace = false;
};

struct MouseButtonState {
  bool left = false;
  bool right = false;
  bool middle = false;
  bool x1 = false;
  bool x2 = false;
};

class InputState {
 public:
  void Reset();
  void UpdatePointer(const MappedWorkspacePoint& mappedWorkspacePoint);
  void UpdateMouseButton(const MouseButtonEvent& mouseButtonEvent);
  void UpdateKeyboardKey(const KeyboardEvent& keyboardEvent);

  [[nodiscard]] const PointerState& Pointer() const noexcept { return pointer_; }
  [[nodiscard]] const MouseButtonState& MouseButtons() const noexcept { return mouseButtons_; }
  [[nodiscard]] bool IsMouseButtonPressed(MouseButton button) const noexcept;
  [[nodiscard]] bool IsKeyPressed(std::uint32_t virtualKey) const noexcept;

 private:
  static constexpr std::size_t kTrackedVirtualKeyCount = 256;

  PointerState pointer_{};
  MouseButtonState mouseButtons_{};
  std::array<bool, kTrackedVirtualKeyCount> keyStates_{};
};

}  // namespace input
}  // namespace supra
