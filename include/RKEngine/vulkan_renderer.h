#pragma once
#include <string>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "renderer.h"

namespace RKEngine
{
  class VulkanRenderer : public Renderer
  {
  public:
    VulkanRenderer(const uint32_t &window_width, const uint32_t window_height, const std::string &window_title);
    ~VulkanRenderer();

    void draw() override;
    bool window_should_close() override;

  private:
    GLFWwindow *m_window;
    uint32_t m_window_width;
    uint32_t m_window_height;
    std::string m_window_title;

    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debug_messenger;

    void create_window();
    void destroy_window();
    void create_instance();
    void destroy_instance();
    void create_debug_messenger();
    void destroy_debug_messenger();
  };
}