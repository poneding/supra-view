#pragma once

#ifdef _WIN32

#include <d3d11.h>

namespace supra::renderer {

class FullscreenQuad {
 public:
  void Draw(ID3D11DeviceContext* context) const;
};

}  // namespace supra::renderer

#else

namespace supra::renderer {

class FullscreenQuad {
 public:
  void Draw(void*) const {}
};

}  // namespace supra::renderer

#endif
