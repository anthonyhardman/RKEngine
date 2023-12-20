#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "RKEngine/vulkan_renderer.h"

#ifdef NDEBUG
constexpr bool enable_validation_layers = false;
#else
constexpr bool enable_validation_layers = true;
#endif

const std::vector<const char *> get_required_extensions();
const std::vector<VkLayerProperties> get_available_layers();
const bool check_validation_layer_support(std::vector<const char *> &validation_layers);
const void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                              VkDebugUtilsMessageTypeFlagsEXT message_type,
                                              const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
                                              void *p_user_data);
VkResult create_debug_utils_messenger_ext(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT *p_create_info,
                                          const VkAllocationCallbacks *p_allocator,
                                          VkDebugUtilsMessengerEXT *p_debug_messenger);
void destroy_debug_utils_messenger_ext(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debug_messenger,
                                       const VkAllocationCallbacks *p_allocator);


RKEngine::VulkanRenderer::VulkanRenderer(const uint32_t &window_width, const uint32_t window_height, const std::string &window_title)
  : m_window_width(window_width), m_window_height(window_height), m_window_title(window_title)
{
  create_window();
  create_instance();
  create_debug_messenger();
}


RKEngine::VulkanRenderer::~VulkanRenderer()
{
  destroy_debug_messenger();
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

  if (enable_validation_layers && !check_validation_layer_support(validation_layers))
  {
    throw std::runtime_error("Validation layers requested, but not available!");
  }

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
  if (enable_validation_layers)
  {
    create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();

    populate_debug_messenger_create_info(debug_create_info);
    create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;
  }
  else
  {
    create_info.enabledLayerCount = 0;
  }

  if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create vulkan instance!");
  }
}

void RKEngine::VulkanRenderer::destroy_instance()
{
  vkDestroyInstance(m_instance, nullptr);
}

void RKEngine::VulkanRenderer::create_debug_messenger()
{
  if (!enable_validation_layers)
  {
    return;
  }

  VkDebugUtilsMessengerCreateInfoEXT create_info;
  populate_debug_messenger_create_info(create_info);

  if (create_debug_utils_messenger_ext(m_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to set up debug messenger!");
  }
}

void RKEngine::VulkanRenderer::destroy_debug_messenger()
{
  if (!enable_validation_layers)
  {
    return;
  }

  destroy_debug_utils_messenger_ext(m_instance, m_debug_messenger, nullptr);
}


void RKEngine::VulkanRenderer::draw()
{
  glfwPollEvents();
}

bool RKEngine::VulkanRenderer::window_should_close()
{
  return glfwWindowShouldClose(m_window);
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

const std::vector<VkLayerProperties> get_available_layers()
{
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
  return available_layers;
}

const bool check_validation_layer_support(std::vector<const char *> &validation_layers)
{
  std::vector<VkLayerProperties> available_layers = get_available_layers();

  return std::all_of(validation_layers.begin(), validation_layers.end(),
                     [&available_layers](const char *layer_name)
                     {
                       return std::find_if(available_layers.begin(), available_layers.end(),
                                           [&layer_name](const VkLayerProperties &layer)
                                           {
                                             return strcmp(layer_name, layer.layerName) == 0;
                                           }) != available_layers.end();
                     });
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                              VkDebugUtilsMessageTypeFlagsEXT message_type,
                                              const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
                                              void *p_user_data)

{
  switch (message_severity)
  {
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    std::clog << "[verbose] " << p_callback_data->pMessage << std::endl;
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    std::clog << "[info] " << p_callback_data->pMessage << std::endl;
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    std::cerr << "[warning] " << p_callback_data->pMessage << std::endl;
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    std::cerr << "[error] " << p_callback_data->pMessage << std::endl;
    break;
  default:
    std::cerr << "[unknown] " << p_callback_data->pMessage << std::endl;
    break;
  }

  return VK_FALSE;
}

const void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info)
{
  create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = debug_callback;
  create_info.pUserData = nullptr;
}

VkResult create_debug_utils_messenger_ext(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT *p_create_info,
                                          const VkAllocationCallbacks *p_allocator,
                                          VkDebugUtilsMessengerEXT *p_debug_messenger)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    return func(instance, p_create_info, p_allocator, p_debug_messenger);
  }
  else
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void destroy_debug_utils_messenger_ext(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debug_messenger,
                                       const VkAllocationCallbacks *p_allocator)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    func(instance, debug_messenger, p_allocator);
  }
}