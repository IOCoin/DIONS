#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include<vector>
struct Frame
{
  std::vector<unsigned char> data;
};
class RenderEngine
{
  public:
    RenderEngine() { }
    virtual ~RenderEngine() { }

    virtual void paint();
    virtual bool opaque();
    virtual const Frame& aspect() = 0;
    virtual const Frame& shade() = 0;

  private:
    std::vector<unsigned char> addr;
};

#endif
