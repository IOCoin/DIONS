#include <stdlib.h>
#include <string>
#include <tuple>
#include <map>
#include <memory>
#include <vector>
#include <iostream>

class Validator
{
public:
  Validator() { }
  ~Validator() { }
  virtual bool validate() = 0;

private:
};

class SMC : public Validator
{
public:
  SMC(int i)
  {
    id = i;
  }
  ~SMC() { }
  virtual bool validate() override
  {
    std::cout << "SMC - id " << id << std::endl;
    return true;
  }

  int id;
};

class Wasm : public Validator
{
public:
  Wasm(int i)
  {
    id = i;
  }
  ~Wasm() { }
  virtual bool validate() override
  {
    std::cout << "Wasm - id " << id << std::endl;
    return true;
  }

  int id;
};

template <typename Validator_T>
class ValidationEngine
{
public:
  ValidationEngine() { }

  void addPlugin(std::unique_ptr<Validator_T>& v);
  void process()
  {
    for(int i=0; i<validators.size(); i++ )
    {
      std::cout << "proc" << std::endl;
      validators[i]->validate();
    }
  }

  std::vector<std::unique_ptr<Validator_T>> validators;
};
template <typename Validator_T>
void ValidationEngine<Validator_T>::addPlugin(std::unique_ptr<Validator_T>& v)
{
  v->validate();
  this->validators.emplace_back(std::move(v));
}


int main()
{
  ValidationEngine<Validator> engine;

  std::unique_ptr<Validator> v1 = std::make_unique<SMC>(1);
  std::unique_ptr<Validator> v2 = std::make_unique<SMC>(2);
  std::unique_ptr<Validator> v3 = std::make_unique<SMC>(3);
  std::unique_ptr<Validator> v4 = std::make_unique<SMC>(4);
  std::unique_ptr<Validator> v5 = std::make_unique<SMC>(5);
  std::unique_ptr<Validator> v6 = std::make_unique<SMC>(6);
  engine.addPlugin(v1);
  engine.addPlugin(v2);
  engine.addPlugin(v3);
  engine.addPlugin(v4);
  engine.addPlugin(v5);
  engine.addPlugin(v6);

  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  
  std::unique_ptr<Validator> v7 = std::make_unique<Wasm>(1);
  std::unique_ptr<Validator> v8 = std::make_unique<Wasm>(2);
  std::unique_ptr<Validator> v9 = std::make_unique<Wasm>(3);
  std::unique_ptr<Validator> v10 = std::make_unique<Wasm>(4);
  std::unique_ptr<Validator> v11 = std::make_unique<Wasm>(5);
  std::unique_ptr<Validator> v12 = std::make_unique<Wasm>(6);
  engine.addPlugin(v7);
  engine.addPlugin(v8);
  engine.addPlugin(v9);
  engine.addPlugin(v10);
  engine.addPlugin(v11);
  engine.addPlugin(v12);

  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

  engine.process();
}
