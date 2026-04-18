#pragma once

#include <cstdint>

#ifdef _WIN32
#include <windows.h>
using SupraUint = UINT;
#else
using SupraUint = std::uint32_t;
#endif

namespace supra::renderer {

enum class ScalingMode : SupraUint {
  Bicubic = 0,
  Lanczos = 1,
};

struct RenderConfig {
  SupraUint outputWidth = 2560;
  SupraUint outputHeight = 1440;
  ScalingMode scalingMode = ScalingMode::Bicubic;
};

struct NormalizedMaskRect {
  SupraUint left = 0;
  SupraUint top = 0;
  SupraUint right = 0;
  SupraUint bottom = 0;
};

struct alignas(16) DownsampleConstants {
  float sourceSize[2];
  float inverseSourceSize[2];
  float targetSize[2];
  float filterMode = 0.0f;
  float maskTopLeft[2] = {0.0f, 0.0f};
  float maskBottomRight[2] = {0.0f, 0.0f};
  float maskEnabled = 0.0f;
  float padding[2] = {0.0f, 0.0f};
};

static_assert(sizeof(DownsampleConstants) % 16 == 0, "Constant buffer size must be 16-byte aligned.");

}  // namespace supra::renderer
