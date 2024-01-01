#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace RKEngine
{
  class VulkanValidation
  {
  public:
    VulkanValidation() = default;
    ~VulkanValidation() = default;
    bool check_validation_layer_support() const;
    void setup_instance_creation_validation(VkInstanceCreateInfo &create_info, VkDebugUtilsMessengerCreateInfoEXT &debug_create_info) const;
    void setup_device_creation_validation(VkDeviceCreateInfo &create_info) const;
    void create_debug_messenger(VkInstance &instance);
    void destroy_debug_messenger(VkInstance &instance);

  private:
    VkDebugUtilsMessengerEXT m_debug_messenger;

    const std::vector<const char *> m_validation_layers = {
        "VK_LAYER_KHRONOS_validation"};
  };
}