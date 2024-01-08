#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "RKEngine/vulkan_renderer.h"

const std::vector<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const bool check_device_extension_support(VkPhysicalDevice device);

RKEngine::VulkanRenderer::VulkanRenderer(const uint32_t &window_width, const uint32_t window_height, const std::string &window_title)
    : m_window_width(window_width), m_window_height(window_height), m_window_title(window_title)
{
  create_window();
  create_instance();
  create_surface();
  m_validation.create_debug_messenger(m_instance);
  pick_physical_device();
  create_logical_device();
  create_swap_chain();
  create_image_views();
}

RKEngine::VulkanRenderer::~VulkanRenderer()
{
  destroy_image_views();
  destroy_swap_chain();
  destroy_logical_device();
  m_validation.destroy_debug_messenger(m_instance);
  destroy_surface();
  destroy_instance();
  destroy_window();
}

void RKEngine::VulkanRenderer::create_window()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  m_window = glfwCreateWindow(m_window_width, m_window_height, m_window_title.c_str(), nullptr, nullptr);
}

void RKEngine::VulkanRenderer::destroy_window()
{
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

void RKEngine::VulkanRenderer::create_instance()
{
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "RKEngine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_3;

  std::vector<const char *> extensions = get_required_extensions();

  std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};

  if (!m_validation.check_validation_layer_support())
  {
    throw std::runtime_error("Validation layers requested, but not available!");
  }

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
  m_validation.setup_instance_creation_validation(create_info, debug_create_info);

  if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create vulkan instance!");
  }
}

void RKEngine::VulkanRenderer::destroy_instance()
{
  vkDestroyInstance(m_instance, nullptr);
}

void RKEngine::VulkanRenderer::draw()
{
  glfwPollEvents();
}

bool RKEngine::VulkanRenderer::window_should_close()
{
  return glfwWindowShouldClose(m_window);
}

std::vector<VkPhysicalDevice> RKEngine::VulkanRenderer::get_list_of_physical_devices() const
{
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
  if (device_count == 0)
  {
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

  return devices;
}

void RKEngine::VulkanRenderer::pick_physical_device()
{
  std::vector<VkPhysicalDevice> devices = get_list_of_physical_devices();

  for (const auto &device : devices)
  {
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);

    if (is_device_suitable(device))
    {
      m_physical_device = device;
      break;
    }
  }
}

void RKEngine::VulkanRenderer::create_logical_device()
{
  QueueFamilyIndices indices = find_queue_family_indices(m_physical_device);
  std::set<uint32_t> unique_queue_families{indices.graphics_family.value(), indices.present_family.value()};
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  float queue_priority = 1.0f;

  for (const auto &queue_family : unique_queue_families)
  {
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }

  VkPhysicalDeviceFeatures device_features = {};

  VkDeviceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.pQueueCreateInfos = queue_create_infos.data();
  create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
  create_info.pEnabledFeatures = &device_features;
  create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
  create_info.ppEnabledExtensionNames = device_extensions.data();
  m_validation.setup_device_creation_validation(create_info);

  if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create logical device!");
  }

  vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0, &m_graphics_queue);
  vkGetDeviceQueue(m_device, indices.present_family.value(), 0, &m_present_queue);
}

void RKEngine::VulkanRenderer::destroy_logical_device()
{
  vkDestroyDevice(m_device, nullptr);
}

void RKEngine::VulkanRenderer::create_surface()
{
  if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create window surface!");
  }
}

void RKEngine::VulkanRenderer::destroy_surface()
{
  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

void RKEngine::VulkanRenderer::create_swap_chain()
{
  SwapChainSupportDetails swap_chain_details = query_swap_chain_support(m_physical_device);

  VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_details.formats);
  VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_details.present_modes);
  VkExtent2D extent = choose_swap_extent(swap_chain_details.capabilities);

  uint32_t image_count = swap_chain_details.capabilities.minImageCount + 1;
  if (swap_chain_details.capabilities.maxImageCount > 0 && image_count > swap_chain_details.capabilities.maxImageCount)
  {
    image_count = swap_chain_details.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = m_surface;
  create_info.minImageCount = image_count;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = find_queue_family_indices(m_physical_device);
  uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

  if (indices.graphics_family != indices.present_family)
  {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  }
  else
  {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
  }

  create_info.preTransform = swap_chain_details.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;
  create_info.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swap_chain) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create swap chain!");
  }

  vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, nullptr);
  m_swap_chain_images.resize(image_count);
  vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, m_swap_chain_images.data());

  m_swap_chain_image_format = surface_format.format;
  m_swap_chain_extent = extent;
}

void RKEngine::VulkanRenderer::destroy_swap_chain()
{
  vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);
}

void RKEngine::VulkanRenderer::create_image_views()
{
  m_swap_chain_image_views.resize(m_swap_chain_images.size());

  for (size_t i = 0; i < m_swap_chain_images.size(); ++i)
  {
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = m_swap_chain_images[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = m_swap_chain_image_format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device, &create_info, nullptr, &m_swap_chain_image_views[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create image views!");
    }
  }
}

void RKEngine::VulkanRenderer::destroy_image_views()
{
  for (auto &image_view : m_swap_chain_image_views)
  {
    vkDestroyImageView(m_device, image_view, nullptr);
  }
}

void RKEngine::VulkanRenderer::create_graphics_pipeline()
{
  
}

const std::vector<const char *> RKEngine::VulkanRenderer::get_required_extensions() const
{
  uint32_t extension_count = 0;
  const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&extension_count);
  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + extension_count);

  std::vector<const char *> validation_extensions = m_validation.get_validation_extensions();
  extensions.insert(extensions.end(), validation_extensions.begin(), validation_extensions.end());

  return extensions;
}

const bool RKEngine::VulkanRenderer::is_device_suitable(VkPhysicalDevice device)
{
  VkPhysicalDeviceProperties device_properties;
  VkPhysicalDeviceFeatures device_features;
  vkGetPhysicalDeviceProperties(device, &device_properties);
  vkGetPhysicalDeviceFeatures(device, &device_features);

  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

  bool correct_device_type = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader;
  bool has_graphics_queue = find_queue_family_indices(device).is_complete();
  bool supports_extensions = check_device_extension_support(device);
  bool swap_chain_adequate = false;
  if (supports_extensions)
  {
    SwapChainSupportDetails swap_chain_support = query_swap_chain_support(device);
    swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
  }

  return correct_device_type && has_graphics_queue && supports_extensions && swap_chain_adequate;
}

const RKEngine::QueueFamilyIndices RKEngine::VulkanRenderer::find_queue_family_indices(VkPhysicalDevice device)
{
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

  RKEngine::QueueFamilyIndices indices;

  for (uint32_t i = 0; i < queue_families.size(); i++)
  {
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      indices.graphics_family = i;
    }

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &present_support);

    if (present_support)
    {
      indices.present_family = i;
    }

    if (indices.is_complete())
    {
      break;
    }
  }

  return indices;
}

const RKEngine::SwapChainSupportDetails RKEngine::VulkanRenderer::query_swap_chain_support(VkPhysicalDevice device)
{
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr);

  if (format_count != 0)
  {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, details.formats.data());
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, nullptr);

  if (present_mode_count != 0)
  {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, details.present_modes.data());
  }

  return details;
}

const VkSurfaceFormatKHR RKEngine::VulkanRenderer::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats)
{
  for (const auto &available_format : available_formats)
  {
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      return available_format;
    }
  }

  return available_formats[0];
}

const VkPresentModeKHR RKEngine::VulkanRenderer::choose_swap_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes)
{
  for (const auto &available_present_mode : available_present_modes)
  {
    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      return available_present_mode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

const VkExtent2D RKEngine::VulkanRenderer::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities)
{
  if (capabilities.currentExtent.width != UINT32_MAX)
  {
    return capabilities.currentExtent;
  }
  else
  {
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actual_extent;
  }
}

const bool check_device_extension_support(VkPhysicalDevice device)
{
  uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

  std::vector<VkExtensionProperties> available_extensions(extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

  std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

  for (const auto &extension : available_extensions)
  {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}
