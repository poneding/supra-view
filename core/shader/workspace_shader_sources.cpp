#include "workspace_shader_sources.h"

#include "workspace_downsample_shader_contract.h"

#include <string>

namespace supra {
namespace shader {

const char* WorkspaceFullscreenVertexShaderSource() {
  return R"(
struct WorkspaceFullscreenVertexOutput {
  float4 position : SV_Position;
  float2 uv : TEXCOORD0;
};

WorkspaceFullscreenVertexOutput WorkspaceFullscreenVS(uint vertexId : SV_VertexID) {
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

  WorkspaceFullscreenVertexOutput output;
  output.position = float4(positions[vertexId], 0.0, 1.0);
  output.uv = uvs[vertexId];
  return output;
}
  )";
}

const char* WorkspaceBicubicDownsamplePixelShaderSource() {
  static const std::string source = std::string(WorkspaceDownsampleConstantBufferHlslSource()) + R"(

Texture2D gLogicalSourceTexture : register(t0);
SamplerState gLogicalSourceSampler : register(s0);

struct WorkspaceFullscreenVertexOutput {
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

float4 WorkspaceBicubicSample(float2 presentationUv) {
  const float2 sourceToPresentationScale = max(gLogicalSourceSize * gInvPresentationTargetSize,
                                                float2(1.0, 1.0));
  const float2 sourcePixel = (presentationUv * gLogicalSourceSize) - 0.5;
  const float2 basePosition = floor(sourcePixel);
  const float2 fractional = sourcePixel - basePosition;

  float4 totalColor = 0.0;
  float totalWeight = 0.0;

  [unroll]
  for (int y = -1; y <= 2; ++y) {
    [unroll]
    for (int x = -1; x <= 2; ++x) {
      const float2 sampleOffset = float2((float)x, (float)y) * sourceToPresentationScale;
      const float2 weightOffset = float2(((float)x - fractional.x) / sourceToPresentationScale.x,
                                         ((float)y - fractional.y) / sourceToPresentationScale.y);
      const float weight = CubicWeight(weightOffset.x) * CubicWeight(weightOffset.y);
      const float2 samplePixel = basePosition + sampleOffset + 0.5;
      const float2 sampleUv = saturate(samplePixel * gInvLogicalSourceSize);
      totalColor += gLogicalSourceTexture.SampleLevel(gLogicalSourceSampler, sampleUv, 0.0) * weight;
      totalWeight += weight;
    }
  }

  return totalColor / max(totalWeight, 0.0001);
}

float4 WorkspaceBicubicDownsamplePS(WorkspaceFullscreenVertexOutput input) : SV_Target {
  const float2 presentationUv = saturate(input.position.xy * gInvPresentationTargetSize);
  return WorkspaceBicubicSample(presentationUv);
}
  )";
  return source.c_str();
}

}  // namespace shader
}  // namespace supra
