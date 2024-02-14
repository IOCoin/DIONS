#include "validation/engine/ValidationEngine.h"

template <class Validator>
void ValidationEngine<Validator>::addPlugin(Validator& v) 
{ 
  this->validators.push_back(std::make_unique<Validator>(v)); 
}
