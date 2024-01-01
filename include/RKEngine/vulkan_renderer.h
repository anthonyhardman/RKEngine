#pragma once
#include <string>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>

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
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    VkDevice m_device;
    VkQueue m_graphics_queue;
    VkDebugUtilsMessengerEXT m_debug_messenger;

    void create_window();
    void destroy_window();
    void create_instance();
    void destroy_instance();
    void create_debug_messenger();
    void destroy_debug_messenger();
    std::vector<VkPhysicalDevice> get_list_of_physical_devices();
    void pick_physical_device();
    void create_logical_device();
    void destroy_logical_device();
  };
}