#include "validation/engine/ValidationEngine.h"

template <class T>
void ValidationEngine<T>::addPlugin(std::unique_ptr<T>& v)
{ 
  this->validators.emplace_back(std::move(v)); 
}
