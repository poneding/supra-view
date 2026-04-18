#include "fullscreen_quad.h"

#ifdef _WIN32

namespace supra::renderer {

void FullscreenQuad::Draw(ID3D11DeviceContext* context) const {
  if (context == nullptr) {
    return;
  }

  context->IASetInputLayout(nullptr);
  context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
  context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->Draw(3, 0);
}

}  // namespace supra::renderer

#endif
