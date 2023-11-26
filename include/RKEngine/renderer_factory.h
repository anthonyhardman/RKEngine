#include <memory>
#include <string>

#include "renderer.h"


namespace RKEngine {
  class RendererFactory {
  public:
    static std::unique_ptr<Renderer> create_renderer();
  };
}