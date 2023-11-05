#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>


namespace RKEngine
{
  class Renderer
  {
  public:
    Renderer(const uint32_t& window_width, const uint32_t& window_height, const std::string& window_title);
    ~Renderer();

    bool window_should_close();
    void draw();

  private:
    GLFWwindow *m_window;
  };
}