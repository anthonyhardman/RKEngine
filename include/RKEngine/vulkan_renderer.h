#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <string>
#include <vulkan/vulkan.h>
#include <vector>

#include "renderer.h"
#include "vulkan_validation.h"

namespace RKEngine
{
  struct QueueFamilyIndices;
  struct SwapChainSupportDetails;

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
    VkQueue m_present_queue;
    VulkanValidation m_validation;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swap_chain;
    std::vector<VkImage> m_swap_chain_images;
    VkFormat m_swap_chain_image_format;
    VkExtent2D m_swap_chain_extent;
    std::vector<VkImageView> m_swap_chain_image_views;

    void create_window();
    void destroy_window();
    void create_instance();
    void destroy_instance();
    void pick_physical_device();
    void create_logical_device();
    void destroy_logical_device();
    void create_surface();
    void destroy_surface();
    void create_swap_chain();
    void destroy_swap_chain();
    void create_image_views();
    void destroy_image_views();
    void create_graphics_pipeline();
    std::vector<VkPhysicalDevice> get_list_of_physical_devices() const;
    const QueueFamilyIndices find_queue_family_indices(VkPhysicalDevice device);
    const bool is_device_suitable(VkPhysicalDevice device);
    const SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);
    const std::vector<const char *> get_required_extensions() const;
    const VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats);
    const VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes);
    const VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities);
  };

  struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete() const
    {
      return graphics_family.has_value() && present_family.has_value();
    }
  };

  struct SwapChainSupportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
  };
}