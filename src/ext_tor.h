#ifndef EXT_TOR
#define EXT_TOR

class ExtTor
{
  public:
    virtual void gr() == 0; 
    virtual void genbase() == 0; 
    virtual void field_embed() == 0; 
    virtual void translate(int,int,int) == 0; 
    virtual void shift(int) == 0; 
    ~virtual ExtTor();
  private:
};

#endif
