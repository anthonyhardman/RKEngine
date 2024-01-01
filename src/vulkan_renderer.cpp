#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "RKEngine/vulkan_renderer.h"

#ifdef NDEBUG
constexpr bool enable_validation_layers = false;
#else
constexpr bool enable_validation_layers = true;
#endif

const std::vector<const char *> get_required_extensions();
const bool is_device_suitable(VkPhysicalDevice device);
const std::optional<size_t> find_queue_family_index(VkPhysicalDevice device, VkQueueFlagBits queue_flag);


RKEngine::VulkanRenderer::VulkanRenderer(const uint32_t &window_width, const uint32_t window_height, const std::string &window_title)
    : m_window_width(window_width), m_window_height(window_height), m_window_title(window_title)
{
  create_window();
  create_instance();
  m_validation.create_debug_messenger(m_instance);
  pick_physical_device();
  create_logical_device();
}

RKEngine::VulkanRenderer::~VulkanRenderer()
{
  destroy_logical_device();
  m_validation.destroy_debug_messenger(m_instance);
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

  if (enable_validation_layers && !m_validation.check_validation_layer_support())
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

std::vector<VkPhysicalDevice> RKEngine::VulkanRenderer::get_list_of_physical_devices()
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
  size_t graphics_queue_family_index = find_queue_family_index(m_physical_device, VK_QUEUE_GRAPHICS_BIT).value();

  VkDeviceQueueCreateInfo queue_create_info = {};
  queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info.queueFamilyIndex = graphics_queue_family_index;
  queue_create_info.queueCount = 1;

  float queue_priority = 1.0f;
  queue_create_info.pQueuePriorities = &queue_priority;

  VkPhysicalDeviceFeatures device_features = {};

  VkDeviceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.pQueueCreateInfos = &queue_create_info;
  create_info.queueCreateInfoCount = 1;
  create_info.pEnabledFeatures = &device_features;
  create_info.enabledExtensionCount = 0;
  m_validation.setup_device_creation_validation(create_info);

  if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create logical device!");
  }

  vkGetDeviceQueue(m_device, graphics_queue_family_index, 0, &m_graphics_queue);
}

void RKEngine::VulkanRenderer::destroy_logical_device()
{
  vkDestroyDevice(m_device, nullptr);
}

const std::vector<const char *> get_required_extensions()
{
  uint32_t extension_count = 0;
  const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&extension_count);
  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + extension_count);

  if (enable_validation_layers)
  {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

const bool is_device_suitable(VkPhysicalDevice device)
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
  bool has_graphics_queue = find_queue_family_index(device, VK_QUEUE_GRAPHICS_BIT).has_value();

  return correct_device_type && has_graphics_queue;
}

const std::optional<size_t> find_queue_family_index(VkPhysicalDevice device, VkQueueFlagBits queue_flag)
{
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

  for (size_t i = 0; i < queue_families.size(); i++)
  {
    if (queue_families[i].queueFlags & queue_flag == queue_flag)
    {
      return i;
    }
  }

  return std::nullopt;
}