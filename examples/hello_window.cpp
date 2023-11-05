#include <RKEngine/renderer.h>


int main()
{
  RKEngine::Renderer renderer(800, 600, "Hello, Window!");
  while (!renderer.window_should_close())
  {
    renderer.draw();
  }
  return 0;
}