#include "workspace_placeholder_scene.h"

#include <algorithm>

#ifdef _WIN32
#include <d3d11.h>
#include <d3d11_1.h>
#include <limits>
#endif

namespace supra::renderer {

namespace {

WorkspaceUint ClampExtent(WorkspaceUint value, WorkspaceUint minValue, WorkspaceUint maxValue) {
  if (maxValue < minValue) {
    maxValue = minValue;
  }

  return std::min(std::max(value, minValue), maxValue);
}

WorkspaceUint ComputeScaledExtent(WorkspaceUint fullExtent, float scale, WorkspaceUint minValue) {
  if (fullExtent == 0) {
    return 0;
  }

  const WorkspaceUint scaledValue = static_cast<WorkspaceUint>(static_cast<float>(fullExtent) * scale);
  return ClampExtent(scaledValue, std::min(minValue, fullExtent), fullExtent);
}

WorkspaceUint ComputeTextDrivenExtent(WorkspaceUint availableWidth, std::size_t textLength,
                                      float minScale, float maxScale) {
  if (availableWidth == 0) {
    return 0;
  }

  const std::size_t normalizedLength = (textLength % 9U) + 3U;
  const float interpolation = static_cast<float>(normalizedLength) / 12.0f;
  const float scale = minScale + ((maxScale - minScale) * interpolation);
  return ComputeScaledExtent(availableWidth, scale, 1U);
}

#ifdef _WIN32
LONG ToLong(UINT value) {
  return value >= static_cast<UINT>(std::numeric_limits<LONG>::max())
             ? std::numeric_limits<LONG>::max()
             : static_cast<LONG>(value);
}

UINT ResolveBound(float bound, UINT extent) {
  if (extent == 0) {
    return 0;
  }

  const float clampedBound = std::min(std::max(bound, 0.0f), 1.0f);
  return static_cast<UINT>(clampedBound * static_cast<float>(extent));
}

D3D11_RECT MakeRect(float left, float top, float right, float bottom, UINT width, UINT height) {
  const UINT resolvedLeft = ResolveBound(left, width);
  const UINT resolvedTop = ResolveBound(top, height);
  const UINT resolvedRight = ResolveBound(right, width);
  const UINT resolvedBottom = ResolveBound(bottom, height);

  return D3D11_RECT{
      ToLong(resolvedLeft),
      ToLong(resolvedTop),
      ToLong(std::max(resolvedLeft, resolvedRight)),
      ToLong(std::max(resolvedTop, resolvedBottom)),
  };
}
#endif

float NormalizeBound(WorkspaceUint value, WorkspaceUint extent) {
  if (extent == 0) {
    return 0.0f;
  }

  const WorkspaceUint clampedValue = std::min(value, extent);
  return static_cast<float>(clampedValue) / static_cast<float>(extent);
}

}  // namespace

void PlaceholderScene::AddRect(float left, float top, float right, float bottom, const float* color) {
  if (color == nullptr || rectCount >= rects.size()) {
    return;
  }

  rects[rectCount++] = PlaceholderRect{
      std::min(std::max(left, 0.0f), 1.0f),
      std::min(std::max(top, 0.0f), 1.0f),
      std::min(std::max(right, left), 1.0f),
      std::min(std::max(bottom, top), 1.0f),
      color,
  };
}

PlaceholderScene BuildPlaceholderScene(
    WorkspaceUint width, WorkspaceUint height,
    const supra::session::WorkspaceRenderResult& renderResult) {
  PlaceholderScene scene;

  const auto& background = renderResult.state.content.background;
  scene.clearColor = {
      background.red,
      background.green,
      background.blue,
      background.alpha,
  };

  if (width == 0 || height == 0) {
    return scene;
  }

  const WorkspaceUint margin = ComputeScaledExtent(std::min(width, height), 0.04f, 8U);
  const WorkspaceUint contentWidth = width > margin * 2U ? width - (margin * 2U) : width;
  const WorkspaceUint contentHeight = height > margin * 2U ? height - (margin * 2U) : height;

  if (contentWidth == 0 || contentHeight == 0) {
    return scene;
  }

  static constexpr float kChromeColor[4] = {0.12f, 0.15f, 0.22f, 1.0f};
  static constexpr float kAccentColor[4] = {0.24f, 0.55f, 0.76f, 1.0f};
  static constexpr float kMutedColor[4] = {0.17f, 0.21f, 0.29f, 1.0f};
  static constexpr float kPanelColor[4] = {0.10f, 0.12f, 0.18f, 1.0f};

  const WorkspaceUint topBarHeight = ComputeScaledExtent(contentHeight, 0.12f, 24U);
  const WorkspaceUint titleBarWidth = ComputeTextDrivenExtent(contentWidth,
                                                              renderResult.state.content.debug.title.size(),
                                                              0.22f, 0.54f);
  scene.AddRect(NormalizeBound(margin, width), NormalizeBound(margin, height),
                NormalizeBound(margin + titleBarWidth, width),
                NormalizeBound(margin + topBarHeight, height), kChromeColor);

  if (renderResult.hasDebugOverlay) {
    const WorkspaceUint debugTop = margin + topBarHeight + margin / 2U;
    const WorkspaceUint debugBarHeight = ComputeScaledExtent(contentHeight, 0.045f, 10U);
    const WorkspaceUint availableWidth = contentWidth > margin ? contentWidth - margin : contentWidth;
    const WorkspaceUint summaryWidth = ComputeTextDrivenExtent(
        availableWidth, renderResult.state.content.debug.summary.size(), 0.28f, 0.78f);
    const WorkspaceUint frameWidth = ComputeTextDrivenExtent(
        availableWidth, renderResult.state.frameNumber == 0U ? 1U : 6U, 0.20f, 0.42f);

    scene.AddRect(NormalizeBound(margin, width), NormalizeBound(debugTop, height),
                  NormalizeBound(margin + summaryWidth, width),
                  NormalizeBound(debugTop + debugBarHeight, height), kAccentColor);
    scene.AddRect(NormalizeBound(margin, width),
                  NormalizeBound(debugTop + debugBarHeight + margin / 2U, height),
                  NormalizeBound(margin + frameWidth, width),
                  NormalizeBound(debugTop + (debugBarHeight * 2U) + margin / 2U, height),
                  kAccentColor);
  }

  if (renderResult.hasVisiblePanel) {
    const WorkspaceUint panelWidth = ComputeScaledExtent(contentWidth, 0.28f, 120U);
    const WorkspaceUint panelLeft = width > margin + panelWidth ? width - margin - panelWidth : margin;
    const WorkspaceUint panelTop = margin;
    const WorkspaceUint panelBottom = height > margin ? height - margin : height;
    const WorkspaceUint panelHeight = panelBottom > panelTop ? panelBottom - panelTop : 0U;
    const WorkspaceUint panelHeaderHeight = ComputeScaledExtent(panelHeight, 0.10f, 22U);
    const WorkspaceUint panelInnerMargin = ComputeScaledExtent(panelWidth, 0.08f, 8U);
    const WorkspaceUint panelContentWidth = panelWidth > panelInnerMargin * 2U
                                                ? panelWidth - (panelInnerMargin * 2U)
                                                : panelWidth;
    const WorkspaceUint panelLineHeight = ComputeScaledExtent(panelHeight, 0.05f, 10U);
    const WorkspaceUint panelTitleWidth = ComputeTextDrivenExtent(
        panelContentWidth, renderResult.state.content.panel.title.size(), 0.42f, 0.82f);
    const WorkspaceUint panelLineOneWidth = ComputeScaledExtent(panelContentWidth, 0.82f, 20U);
    const WorkspaceUint panelLineTwoWidth = ComputeScaledExtent(panelContentWidth, 0.66f, 20U);
    const WorkspaceUint panelLineThreeWidth = ComputeScaledExtent(panelContentWidth, 0.74f, 20U);

    scene.AddRect(NormalizeBound(panelLeft, width), NormalizeBound(panelTop, height),
                  NormalizeBound(panelLeft + panelWidth, width),
                  NormalizeBound(panelBottom, height), kPanelColor);
    scene.AddRect(NormalizeBound(panelLeft + panelInnerMargin, width),
                  NormalizeBound(panelTop + panelInnerMargin, height),
                  NormalizeBound(panelLeft + panelInnerMargin + panelTitleWidth, width),
                  NormalizeBound(panelTop + panelInnerMargin + panelHeaderHeight, height),
                  kMutedColor);
    scene.AddRect(NormalizeBound(panelLeft + panelInnerMargin, width),
                  NormalizeBound(panelTop + panelHeaderHeight + panelInnerMargin * 2U, height),
                  NormalizeBound(panelLeft + panelInnerMargin + panelLineOneWidth, width),
                  NormalizeBound(panelTop + panelHeaderHeight + panelInnerMargin * 2U + panelLineHeight,
                                 height),
                  kChromeColor);
    scene.AddRect(NormalizeBound(panelLeft + panelInnerMargin, width),
                  NormalizeBound(panelTop + panelHeaderHeight + panelInnerMargin * 3U + panelLineHeight,
                                 height),
                  NormalizeBound(panelLeft + panelInnerMargin + panelLineTwoWidth, width),
                  NormalizeBound(panelTop + panelHeaderHeight + panelInnerMargin * 3U + panelLineHeight * 2U,
                                 height),
                  kChromeColor);
    scene.AddRect(NormalizeBound(panelLeft + panelInnerMargin, width),
                  NormalizeBound(panelTop + panelHeaderHeight + panelInnerMargin * 4U + panelLineHeight * 2U,
                                 height),
                  NormalizeBound(panelLeft + panelInnerMargin + panelLineThreeWidth, width),
                  NormalizeBound(panelTop + panelHeaderHeight + panelInnerMargin * 4U + panelLineHeight * 3U,
                                 height),
                  kChromeColor);
  }

  return scene;
}

#ifdef _WIN32
void DrawPlaceholderScene(ID3D11DeviceContext1* context1,
                          ID3D11RenderTargetView* renderTargetView,
                          UINT targetWidth,
                          UINT targetHeight,
                          const PlaceholderScene& scene) {
  if (context1 == nullptr || renderTargetView == nullptr || targetWidth == 0 || targetHeight == 0) {
    return;
  }

  for (std::size_t index = 0; index < scene.rectCount; ++index) {
    const auto& rect = scene.rects[index];
    const D3D11_RECT bounds =
        MakeRect(rect.left, rect.top, rect.right, rect.bottom, targetWidth, targetHeight);
    context1->ClearView(renderTargetView, rect.color, &bounds, 1);
  }
}
#endif

}  // namespace supra::renderer
