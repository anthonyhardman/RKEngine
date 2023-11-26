#pragma once

namespace RKEngine
{
  class Renderer
  {
  public:
    virtual void draw() = 0;
    virtual bool window_should_close() = 0;
  };
}