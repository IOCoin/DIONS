#ifndef _BITCOINALERT_H_
#define _BITCOINALERT_H_ 1

#include <set>
#include <string>

#include "core/uint256.h"
#include "core/Util.h"

class CNode;







class CUnsignedAlert
{
public:
  int nVersion;
  int64_t nRelayUntil;
  int64_t nExpiration;
  int nID;
  int nCancel;
  std::set<int> setCancel;
  int nMinVer;
  int nMaxVer;
  std::set<std::string> setSubVer;
  int nPriority;


  std::string strComment;
  std::string strStatusBar;
  std::string strReserved;

  IMPLEMENT_SERIALIZE
  (
    READWRITE(this->nVersion);
    nVersion = this->nVersion;
    READWRITE(nRelayUntil);
    READWRITE(nExpiration);
    READWRITE(nID);
    READWRITE(nCancel);
    READWRITE(setCancel);
    READWRITE(nMinVer);
    READWRITE(nMaxVer);
    READWRITE(setSubVer);
    READWRITE(nPriority);

    READWRITE(strComment);
    READWRITE(strStatusBar);
    READWRITE(strReserved);
  )

  void SetNull();

  std::string ToString() const;
  void print() const;
};


class CAlert : public CUnsignedAlert
{
public:
  std::vector<unsigned char> vchMsg;
  std::vector<unsigned char> vchSig;

  CAlert()
  {
    SetNull();
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(vchMsg);
    READWRITE(vchSig);
  )

  void SetNull();
  bool IsNull() const;
  uint256 GetHash() const;
  bool IsInEffect() const;
  bool Cancels(const CAlert& alert) const;
  bool AppliesTo(int nVersion, std::string strSubVerIn) const;
  bool AppliesToMe() const;
  bool RelayTo(CNode* pnode) const;
  bool CheckSignature() const;
  bool ProcessAlert(bool fThread = true);




  static CAlert getAlertByHash(const uint256 &hash);
};

#endif
