#ifndef EXT_TOR
#define EXT_TOR

#include<vector>

class f
{
  public:
    f() { }
    f(string u, string p, int el, string ref) : 
      u(u), point(p), el(el), ref(ref) { }

    bool operator==(const f& o) const { return this->u == o.u; }
    const f&  operator=(const f& o) 
    { 
      this->u = o.u; 
      this->point = o.point; 
      this->el = o.el; 
      this->ref = o.ref; 
      return *this;
    }

   void m(const string& s) 
   {
     this->u = s;
   }

   void r() const
   {
     printf("%s\n", this->u.c_str());
     printf("%s\n", this->point.c_str());
     printf("%d\n", this->el);
     printf("%s\n", this->ref.c_str());
   }

  int operator()() const
  {
    return this->el;
  }

  private:
    string u;
    string point; 
    int el;
    string ref;
};

class Hbook
{
  public:
    Hbook() { }

    bool set(const f& o, f& n) 
    {
      printf("set\n");
      o.r();
      for(int a=0; a<v.size(); a++)
      {
        if((n=(f)v[a]) == o) 
        {
          return false;
        }
      }
      
      printf("setting\n");
      v.push_back(o);
      el=o();
      return true;  
    }

    bool l(const f& s, f& n) const
    {
      for(int a=0; a<v.size(); a++)
      {
        if((n=(f)v[a]) == s) 
        {
          return true;
        }
      }
      return false;
    }

    int operator()() const
    {
      return el;
    }

    void r() const
    {
      printf("Hbook begin :\n");
      for(int a=0; a<v.size(); a++)
        v[a].r();      
      printf("Hbook end\n");
    }

  private:
    int el;
    vector<f> v;
};

#endif
