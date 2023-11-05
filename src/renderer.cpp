#include "RkEngine/renderer.h"

namespace RKEngine
{
  Renderer::Renderer(const uint32_t& window_width, const uint32_t& window_height, const std::string& window_title)
  {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(window_width, window_height, window_title.c_str(), nullptr, nullptr);
  }

  Renderer::~Renderer()
  {
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }

  bool Renderer::window_should_close()
  {
    return glfwWindowShouldClose(m_window);
  }

  void Renderer::draw()
  {
    glfwPollEvents();
  }
}