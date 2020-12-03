#ifndef RELAY_ENGINE_H
#define RELAY_ENGINE_H

#include<vector>

class RelayEngine
{
  public:
    RelayEngine() { }

    virtual void relayHandler(RelayEngine& r);
    virtual void execute();

  private:
    std::vector<unsigned char> addr;
};

#endif
