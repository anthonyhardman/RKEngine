#include "RKEngine/renderer_factory.h"
#include <GLFW/glfw3.h>
#include <iostream>


int main()
{
  auto renderer = RKEngine::RendererFactory::create_renderer();

  while (!renderer->window_should_close())
  {
    renderer->draw();
  }
  
  return 0;
}