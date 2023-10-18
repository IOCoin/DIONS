#ifndef BITCOIN_CHECKPOINT_H
#define BITCOIN_CHECKPOINT_H 

#include <map>
#include "Net.h"
#include "core/Util.h"

#ifdef WIN32
#undef STRICT
#undef PERMISSIVE
#undef ADVISORY
#endif

class uint256;
class CBlockIndex;
class CSyncCheckpoint;




namespace Checkpoints
{

enum CPMode
{

  STRICT = 0,

  ADVISORY = 1,

  PERMISSIVE = 2
};


bool CheckHardened(int nHeight, const uint256& hash);


int GetTotalBlocksEstimate();


CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex);

extern uint256 hashSyncCheckpoint;
extern CSyncCheckpoint checkpointMessage;
extern uint256 hashInvalidCheckpoint;
extern CCriticalSection cs_hashSyncCheckpoint;

CBlockIndex* GetLastSyncCheckpoint();
bool WriteSyncCheckpoint(const uint256& hashCheckpoint);
bool AcceptPendingSyncCheckpoint();
uint256 AutoSelectSyncCheckpoint();
bool CheckSync(const uint256& hashBlock, const CBlockIndex* pindexPrev);
bool WantedByPendingSyncCheckpoint(uint256 hashBlock);
bool ResetSyncCheckpoint();
void AskForPendingSyncCheckpoint(CNode* pfrom);
bool SetCheckpointPrivKey(std::string strPrivKey);
bool SendSyncCheckpoint(uint256 hashCheckpoint);
bool IsMatureSyncCheckpoint();
}


class CUnsignedSyncCheckpoint
{
public:
  int nVersion;
  uint256 hashCheckpoint;

  IMPLEMENT_SERIALIZE
  (
    READWRITE(this->nVersion);
    nVersion = this->nVersion;
    READWRITE(hashCheckpoint);
  )

  void SetNull()
  {
    nVersion = 1;
    hashCheckpoint = 0;
  }

  std::string ToString() const
  {
    return strprintf(
             "CSyncCheckpoint(\n"
             "    nVersion       = %d\n"
             "    hashCheckpoint = %s\n"
             ")\n",
             nVersion,
             hashCheckpoint.ToString().c_str());
  }

  void print() const
  {
    printf("%s", ToString().c_str());
  }
};

class CSyncCheckpoint : public CUnsignedSyncCheckpoint
{
public:
  static const std::string strMasterPubKey;
  static std::string strMasterPrivKey;

  std::vector<unsigned char> vchMsg;
  std::vector<unsigned char> vchSig;

  CSyncCheckpoint()
  {
    SetNull();
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(vchMsg);
    READWRITE(vchSig);
  )

  void SetNull()
  {
    CUnsignedSyncCheckpoint::SetNull();
    vchMsg.clear();
    vchSig.clear();
  }

  bool IsNull() const
  {
    return (hashCheckpoint == 0);
  }

  uint256 GetHash() const
  {
    return SerializeHash(*this);
  }

  bool RelayTo(CNode* pnode) const
  {

    if (pnode->hashCheckpointKnown != hashCheckpoint)
    {
      pnode->hashCheckpointKnown = hashCheckpoint;
      pnode->PushMessage("checkpoint", *this);
      return true;
    }
    return false;
  }

  bool CheckSignature();
  bool ProcessSyncCheckpoint(CNode* pfrom);
};

#endif
