#include "workspace_placeholder_scene.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

using supra::renderer::BuildPlaceholderScene;
using supra::renderer::PlaceholderScene;
using supra::session::WorkspaceRenderResult;
using supra::session::WorkspaceResolution;

constexpr float kBoundsTolerance = 0.005f;

void Expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(1);
  }
}

bool NearlyEqual(float left, float right, float tolerance = kBoundsTolerance) {
  return std::fabs(left - right) <= tolerance;
}

WorkspaceRenderResult MakeRenderResult() {
  WorkspaceRenderResult renderResult;
  renderResult.state.resolution = WorkspaceResolution{{1280, 720}, {1920, 1080}};
  renderResult.state.content.debug.title = "Target-independent placeholder scene";
  renderResult.state.content.debug.summary =
      "Logical pass output should be reusable across presentation targets.";
  renderResult.state.content.panel.title = "Inspector";
  renderResult.state.frameNumber = 42U;
  renderResult.hasDebugOverlay = true;
  renderResult.hasVisiblePanel = true;
  return renderResult;
}

void ExpectNormalizedBounds(const PlaceholderScene& scene) {
  for (std::size_t index = 0; index < scene.rectCount; ++index) {
    const auto& rect = scene.rects[index];
    Expect(rect.left >= 0.0f && rect.left <= 1.0f, "Rect left should be normalized.");
    Expect(rect.top >= 0.0f && rect.top <= 1.0f, "Rect top should be normalized.");
    Expect(rect.right >= rect.left && rect.right <= 1.0f,
           "Rect right should be normalized and ordered.");
    Expect(rect.bottom >= rect.top && rect.bottom <= 1.0f,
           "Rect bottom should be normalized and ordered.");
  }
}

void ExpectEquivalentScenes(const PlaceholderScene& left, const PlaceholderScene& right) {
  Expect(left.rectCount == right.rectCount,
         "Same-aspect placeholder scenes should keep the same rect count.");

  for (std::size_t index = 0; index < left.rectCount; ++index) {
    const auto& leftRect = left.rects[index];
    const auto& rightRect = right.rects[index];
    Expect(NearlyEqual(leftRect.left, rightRect.left), "Rect left should scale independently.");
    Expect(NearlyEqual(leftRect.top, rightRect.top), "Rect top should scale independently.");
    Expect(NearlyEqual(leftRect.right, rightRect.right),
           "Rect right should scale independently.");
    Expect(NearlyEqual(leftRect.bottom, rightRect.bottom),
           "Rect bottom should scale independently.");
  }
}

}  // namespace

int main() {
  const WorkspaceRenderResult renderResult = MakeRenderResult();
  const PlaceholderScene logical720p = BuildPlaceholderScene(1280, 720, renderResult);
  const PlaceholderScene logical1440p = BuildPlaceholderScene(2560, 1440, renderResult);

  ExpectNormalizedBounds(logical720p);
  ExpectNormalizedBounds(logical1440p);
  ExpectEquivalentScenes(logical720p, logical1440p);
  return 0;
}
