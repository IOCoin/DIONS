#ifndef DIONS_INPUTPOINT_H
#define DIONS_INPUTPOINT_H

class CInPoint
{
public:
  CTransaction* ptx;
  unsigned int n;

  CInPoint()
  {
    SetNull();
  }
  CInPoint(CTransaction* ptxIn, unsigned int nIn)
  {
    ptx = ptxIn;
    n = nIn;
  }
  void SetNull()
  {
    ptx = NULL;
    n = (unsigned int) -1;
  }
  bool IsNull() const
  {
    return (ptx == NULL && n == (unsigned int) -1);
  }
};

#endif
