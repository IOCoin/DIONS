#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include<vector>

class RenderEngine
{
  public:
    RenderEngine() { }
    virtual ~RenderEngine() { }

    virtual void paint();

  private:
    std::vector<unsigned char> addr;
};

#endif
