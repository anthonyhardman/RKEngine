#include "RKEngine/renderer_factory.h"
#include "RKEngine/vulkan_renderer.h"

namespace RKEngine {
  std::unique_ptr<Renderer> RendererFactory::create_renderer() {
    return std::make_unique<VulkanRenderer>(800, 600, "Hello Window");
  }
}