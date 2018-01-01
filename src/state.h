#ifndef STATE_H
#define STATE_H

#include <string>
#include <vector>

extern unsigned int LR_SHIFT__[];

class State
{
  public:
    static const std::string ATOMIC;
    static const std::string GROUND;
    static const std::string ION;

    State()
    {
    };

    State(std::string k) 
    { 
      STATE_.push_back(State::ATOMIC);
      STATE_.push_back(State::GROUND); 
      STATE_.push_back(State::ION);

      this->m_ = k; 
    }

    const std::string operator()() 
    { 
      for(int i=0; i<STATE_.size(); i++)
      {
        if(this->m_ == STATE_[i])
        {
          return STATE_[i];
        }
      }
      
      return State::ION;
    }

  private:
    std::vector<std::string> STATE_;  
    std::string m_;
    int delta_;
};


#endif
