#pragma once

#include "../session/workspace_session.h"

#include <array>
#include <cstddef>

namespace supra::renderer {

#ifdef _WIN32
struct ID3D11DeviceContext1;
struct ID3D11RenderTargetView;
#endif

struct PlaceholderRect {
  float left = 0.0f;
  float top = 0.0f;
  float right = 0.0f;
  float bottom = 0.0f;
  const float* color = nullptr;
};

struct PlaceholderScene {
  std::array<float, 4> clearColor{};
  std::array<PlaceholderRect, 8> rects{};
  std::size_t rectCount = 0;

  void AddRect(float left, float top, float right, float bottom, const float* color);
};

PlaceholderScene BuildPlaceholderScene(WorkspaceUint width, WorkspaceUint height,
                                       const supra::session::WorkspaceRenderResult& renderResult);

#ifdef _WIN32
void DrawPlaceholderScene(ID3D11DeviceContext1* context1,
                          ID3D11RenderTargetView* renderTargetView,
                          UINT targetWidth,
                          UINT targetHeight,
                          const PlaceholderScene& scene);
#endif

}  // namespace supra::renderer
