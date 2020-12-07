#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include<vector>

class RenderEngine
{
  public:
    RenderEngine() { }
    virtual ~RenderEngine() { }

    virtual void paint();
    virtual bool opaque();

  private:
    std::vector<unsigned char> addr;
};

#endif
