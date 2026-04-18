#include "shader_sources.h"

namespace supra::shader {

const char* FullscreenVertexShaderSource() {
  return R"(
struct VSOutput {
  float4 position : SV_Position;
  float2 uv : TEXCOORD0;
};

VSOutput FullscreenVS(uint vertexId : SV_VertexID) {
  float2 positions[3] = {
    float2(-1.0, -1.0),
    float2(-1.0, 3.0),
    float2(3.0, -1.0)
  };

  float2 uvs[3] = {
    float2(0.0, 1.0),
    float2(0.0, -1.0),
    float2(2.0, 1.0)
  };

  VSOutput output;
  output.position = float4(positions[vertexId], 0.0, 1.0);
  output.uv = uvs[vertexId];
  return output;
}
  )";
}

const char* CopyPixelShaderSource() {
  return R"(
Texture2D gSourceTexture : register(t0);
SamplerState gSourceSampler : register(s0);

struct VSOutput {
  float4 position : SV_Position;
  float2 uv : TEXCOORD0;
};

float4 CopyPS(VSOutput input) : SV_Target {
  return gSourceTexture.SampleLevel(gSourceSampler, input.uv, 0.0);
}
  )";
}

const char* BicubicPixelShaderSource() {
  return R"(
cbuffer DownsampleConstants : register(b0) {
  float2 gSourceSize;
  float2 gInvSourceSize;
  float2 gTargetSize;
  float gFilterMode;
  float3 gPadding;
};

Texture2D gSourceTexture : register(t0);
SamplerState gSourceSampler : register(s0);

struct VSOutput {
  float4 position : SV_Position;
  float2 uv : TEXCOORD0;
};

float CubicWeight(float x) {
  const float a = -0.5;
  x = abs(x);

  if (x <= 1.0) {
    return ((a + 2.0) * x * x * x) - ((a + 3.0) * x * x) + 1.0;
  }

  if (x < 2.0) {
    return (a * x * x * x) - (5.0 * a * x * x) + (8.0 * a * x) - (4.0 * a);
  }

  return 0.0;
}

float4 BicubicSample(float2 uv) {
  float2 texelPosition = uv * gSourceSize - 0.5;
  float2 basePosition = floor(texelPosition);
  float2 fractional = texelPosition - basePosition;

  float4 totalColor = 0.0;
  float totalWeight = 0.0;

  [unroll]
  for (int y = -1; y <= 2; ++y) {
    [unroll]
    for (int x = -1; x <= 2; ++x) {
      float weight = CubicWeight((float)x - fractional.x) * CubicWeight((float)y - fractional.y);
      float2 sampleUv = (basePosition + float2((float)x, (float)y) + 0.5) * gInvSourceSize;
      totalColor += gSourceTexture.SampleLevel(gSourceSampler, sampleUv, 0.0) * weight;
      totalWeight += weight;
    }
  }

  return totalColor / max(totalWeight, 0.0001);
}

float4 BicubicPS(VSOutput input) : SV_Target {
  // gFilterMode keeps the constant buffer Lanczos-ready.
  return BicubicSample(input.uv);
}
  )";
}

}  // namespace supra::shader
