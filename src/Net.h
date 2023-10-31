#pragma once

#include <deque>
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <openssl/rand.h>

#ifndef WIN32
#include <arpa/inet.h>
#endif

#include "MrUset.h"
#include "NetBase.h"
#include "Protocol.h"
#include "AddrMan.h"

class CRequestTracker;
class CNode;
class CBlockIndex;
extern int nBestHeight;



inline unsigned int ReceiveFloodSize()
{
  return 1000*GetArg("-maxreceivebuffer", 5*1000);
}
inline unsigned int SendBufferSize()
{
  return 1000*GetArg("-maxsendbuffer", 1*1000);
}

void AddOneShot(std::string strDest);
bool RecvLine(SOCKET hSocket, std::string& strLine);
bool GetMyExternalIP(CNetAddr& ipRet);
void AddressCurrentlyConnected(const CService& addr);
CNode* FindNode(const CNetAddr& ip);
CNode* FindNode(const CService& ip);
CNode* ConnectNode(CAddress addrConnect, const char *strDest = NULL);
void MapPort();
unsigned short GetListenPort();
bool BindListenPort(const CService &bindAddr, std::string& strError=REF(std::string()));
void StartNode(void* parg);
bool StopNode();
void SocketSendData(CNode *pnode);

enum
{
  LOCAL_NONE,
  LOCAL_IF,
  LOCAL_BIND,
  LOCAL_IRC,
  LOCAL_HTTP,
  LOCAL_MANUAL,

  LOCAL_MAX
};

void SetLimited(enum Network net, bool fLimited = true);
bool IsLimited(enum Network net);
bool IsLimited(const CNetAddr& addr);
bool AddLocal(const CService& addr, int nScore = LOCAL_NONE);
bool AddLocal(const CNetAddr& addr, int nScore = LOCAL_NONE);
bool SeenLocal(const CService& addr);
bool IsLocal(const CService& addr);
bool GetLocal(CService &addr, const CNetAddr *paddrPeer = NULL);
bool IsReachable(const CNetAddr &addr);
void SetReachable(enum Network net, bool fFlag = true);
CAddress GetLocalAddress(const CNetAddr *paddrPeer = NULL);


enum
{
  MSG_TX = 1,
  MSG_BLOCK,
};

class CRequestTracker
{
public:
  void (*fn)(void*, CDataStream&);
  void* param1;

  explicit CRequestTracker(void (*fnIn)(void*, CDataStream&)=NULL, void* param1In=NULL)
  {
    fn = fnIn;
    param1 = param1In;
  }

  bool IsNull()
  {
    return fn == NULL;
  }
};



enum threadId
{
  THREAD_SOCKETHANDLER,
  THREAD_OPENCONNECTIONS,
  THREAD_MESSAGEHANDLER,
  THREAD_RPCLISTENER,
  THREAD_DNSSEED,
  THREAD_ADDEDCONNECTIONS,
  THREAD_DUMPADDRESS,
  THREAD_RPCHANDLER,
  THREAD_STAKE_MINER,

  THREAD_MAX
};

extern bool fDiscover;
extern bool fUseUPnP;
extern uint64_t nLocalServices;
extern uint64_t nLocalHostNonce;
extern CAddress addrSeenByPeer;
extern boost::array<int, THREAD_MAX> vnThreadsRunning;
extern CAddrMan addrman;

extern std::vector<CNode*> vNodes;
extern CCriticalSection cs_vNodes;
extern std::map<CInv, CDataStream> mapRelay;
extern std::deque<std::pair<int64_t, CInv> > vRelayExpiration;
extern CCriticalSection cs_mapRelay;
extern std::map<CInv, int64_t> mapAlreadyAskedFor;




class CNodeStats
{
public:
  uint64_t nServices;
  int64_t nLastSend;
  int64_t nLastRecv;
  int64_t nTimeConnected;
  std::string addrName;
  int nVersion;
  std::string strSubVer;
  bool fInbound;
  int nStartingHeight;
  int nMisbehavior;
};




class CNetMessage
{
public:
  bool in_data;

  CDataStream hdrbuf;
  CMessageHeader hdr;
  unsigned int nHdrPos;

  CDataStream vRecv;
  unsigned int nDataPos;

  CNetMessage(int nTypeIn, int nVersionIn) : hdrbuf(nTypeIn, nVersionIn), vRecv(nTypeIn, nVersionIn)
  {
    hdrbuf.resize(24);
    in_data = false;
    nHdrPos = 0;
    nDataPos = 0;
  }

  bool complete() const
  {
    if (!in_data)
    {
      return false;
    }
    return (hdr.nMessageSize == nDataPos);
  }

  void SetVersion(int nVersionIn)
  {
    hdrbuf.SetVersion(nVersionIn);
    vRecv.SetVersion(nVersionIn);
  }

  int readHeader(const char *pch, unsigned int nBytes);
  int readData(const char *pch, unsigned int nBytes);
};






class CNode
{
public:

  uint64_t nServices;
  SOCKET hSocket;
  CDataStream ssSend;
  size_t nSendSize;
  size_t nSendOffset;
  std::deque<CSerializeData> vSendMsg;
  CCriticalSection cs_vSend;

  std::deque<CNetMessage> vRecvMsg;
  CCriticalSection cs_vRecvMsg;
  int nRecvVersion;

  int64_t nLastSend;
  int64_t nLastRecv;
  int64_t nLastSendEmpty;
  int64_t nTimeConnected;
  CAddress addr;
  std::string addrName;
  CService addrLocal;
  int nVersion;
  std::string strSubVer;
  bool fOneShot;
  bool fClient;
  bool fInbound;
  bool fNetworkNode;
  bool fSuccessfullyConnected;
  bool fDisconnect;
  CSemaphoreGrant grantOutbound;
  int nRefCount;
protected:



  static std::map<CNetAddr, int64_t> setBanned;
  static CCriticalSection cs_setBanned;
  int nMisbehavior;

public:
  std::map<uint256, CRequestTracker> mapRequests;
  CCriticalSection cs_mapRequests;
  uint256 hashContinue;
  CBlockIndex* pindexLastGetBlocksBegin;
  uint256 hashLastGetBlocksEnd;
  int nStartingHeight;


  std::vector<CAddress> vAddrToSend;
  mruset<CAddress> setAddrKnown;
  bool fGetAddr;
  std::set<uint256> setKnown;
  uint256 hashCheckpointKnown;


  mruset<CInv> setInventoryKnown;
  std::vector<CInv> vInventoryToSend;
  CCriticalSection cs_inventory;
  std::multimap<int64_t, CInv> mapAskFor;

  CNode(SOCKET hSocketIn, CAddress addrIn, std::string addrNameIn = "", bool fInboundIn=false) : ssSend(SER_NETWORK, INIT_PROTO_VERSION), setAddrKnown(5000)
  {
    nServices = 0;
    hSocket = hSocketIn;
    nRecvVersion = INIT_PROTO_VERSION;
    nLastSend = 0;
    nLastRecv = 0;
    nLastSendEmpty = GetTime();
    nTimeConnected = GetTime();
    addr = addrIn;
    addrName = addrNameIn == "" ? addr.ToStringIPPort() : addrNameIn;
    nVersion = 0;
    strSubVer = "";
    fOneShot = false;
    fClient = false;
    fInbound = fInboundIn;
    fNetworkNode = false;
    fSuccessfullyConnected = false;
    fDisconnect = false;
    nRefCount = 0;
    nSendSize = 0;
    nSendOffset = 0;
    hashContinue = 0;
    pindexLastGetBlocksBegin = 0;
    hashLastGetBlocksEnd = 0;
    nStartingHeight = -1;
    fGetAddr = false;
    nMisbehavior = 0;
    hashCheckpointKnown = 0;
    setInventoryKnown.max_size(SendBufferSize() / 1000);


    if (hSocket != INVALID_SOCKET && !fInbound)
    {
      PushVersion();
    }
  }

  ~CNode()
  {
    if (hSocket != INVALID_SOCKET)
    {
      closesocket(hSocket);
      hSocket = INVALID_SOCKET;
    }
  }

private:
  CNode(const CNode&);
  void operator=(const CNode&);
public:


  int GetRefCount()
  {
    assert(nRefCount >= 0);
    return nRefCount;
  }


  unsigned int GetTotalRecvSize()
  {
    unsigned int total = 0;
    BOOST_FOREACH(const CNetMessage &msg, vRecvMsg)
    total += msg.vRecv.size() + 24;
    return total;
  }


  bool ReceiveMsgBytes(const char *pch, unsigned int nBytes);


  void SetRecvVersion(int nVersionIn)
  {
    nRecvVersion = nVersionIn;
    BOOST_FOREACH(CNetMessage &msg, vRecvMsg)
    msg.SetVersion(nVersionIn);
  }

  CNode* AddRef()
  {
    nRefCount++;
    return this;
  }

  void Release()
  {
    nRefCount--;
  }



  void AddAddressKnown(const CAddress& addr)
  {
    setAddrKnown.insert(addr);
  }

  void PushAddress(const CAddress& addr)
  {



    if (addr.IsValid() && !setAddrKnown.count(addr))
    {
      vAddrToSend.push_back(addr);
    }
  }


  void AddInventoryKnown(const CInv& inv)
  {
    {
      LOCK(cs_inventory);
      setInventoryKnown.insert(inv);
    }
  }

  void PushInventory(const CInv& inv)
  {
    {
      LOCK(cs_inventory);
      if (!setInventoryKnown.count(inv))
      {
        vInventoryToSend.push_back(inv);
      }
    }
  }

  void AskFor(const CInv& inv)
  {


    int64_t& nRequestTime = mapAlreadyAskedFor[inv];
    if (fDebugNet)
    {
      printf("askfor %s   %" PRId64 " (%s)\n", inv.ToString().c_str(), nRequestTime, DateTimeStrFormat("%H:%M:%S", nRequestTime/1000000).c_str());
    }


    int64_t nNow = (GetTime() - 1) * 1000000;
    static int64_t nLastTime;
    ++nLastTime;
    nNow = std::max(nNow, nLastTime);
    nLastTime = nNow;


    nRequestTime = std::max(nRequestTime + 2 * 60 * 1000000, nNow);
    mapAskFor.insert(std::make_pair(nRequestTime, inv));
  }



  void BeginMessage(const char* pszCommand)
  {
    ENTER_CRITICAL_SECTION(cs_vSend);
    assert(ssSend.size() == 0);
    ssSend << CMessageHeader(pszCommand, 0);
    if (fDebug)
    {
      printf("sending: %s ", pszCommand);
    }
  }

  void AbortMessage()
  {
    ssSend.clear();

    LEAVE_CRITICAL_SECTION(cs_vSend);

    if (fDebug)
    {
      printf("(aborted)\n");
    }
  }

  void EndMessage()
  {
    if (mapArgs.count("-dropmessagestest") && GetRand(atoi(mapArgs["-dropmessagestest"])) == 0)
    {
      printf("dropmessages DROPPING SEND MESSAGE\n");
      AbortMessage();
      return;
    }

    if (ssSend.size() == 0)
    {
      return;
    }


    unsigned int nSize = ssSend.size() - CMessageHeader::HEADER_SIZE;
    memcpy((char*)&ssSend[CMessageHeader::MESSAGE_SIZE_OFFSET], &nSize, sizeof(nSize));


    uint256 hash = Hash(ssSend.begin() + CMessageHeader::HEADER_SIZE, ssSend.end());
    unsigned int nChecksum = 0;
    memcpy(&nChecksum, &hash, sizeof(nChecksum));
    assert(ssSend.size () >= CMessageHeader::CHECKSUM_OFFSET + sizeof(nChecksum));
    memcpy((char*)&ssSend[CMessageHeader::CHECKSUM_OFFSET], &nChecksum, sizeof(nChecksum));

    if (fDebug)
    {
      printf("(%d bytes)\n", nSize);
    }

    std::deque<CSerializeData>::iterator it = vSendMsg.insert(vSendMsg.end(), CSerializeData());
    ssSend.GetAndClear(*it);
    nSendSize += (*it).size();


    if (it == vSendMsg.begin())
    {
      SocketSendData(this);
    }

    LEAVE_CRITICAL_SECTION(cs_vSend);
  }

  void PushVersion();


  void PushMessage(const char* pszCommand)
  {
    try
    {
      BeginMessage(pszCommand);
      EndMessage();
    }
    catch (...)
    {
      AbortMessage();
      throw;
    }
  }

  template<typename T1>
  void PushMessage(const char* pszCommand, const T1& a1)
  {
    try
    {
      BeginMessage(pszCommand);
      ssSend << a1;
      EndMessage();
    }
    catch (...)
    {
      AbortMessage();
      throw;
    }
  }

  template<typename T1, typename T2>
  void PushMessage(const char* pszCommand, const T1& a1, const T2& a2)
  {
    try
    {
      BeginMessage(pszCommand);
      ssSend << a1 << a2;
      EndMessage();
    }
    catch (...)
    {
      AbortMessage();
      throw;
    }
  }

  template<typename T1, typename T2, typename T3>
  void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3)
  {
    try
    {
      BeginMessage(pszCommand);
      ssSend << a1 << a2 << a3;
      EndMessage();
    }
    catch (...)
    {
      AbortMessage();
      throw;
    }
  }

  template<typename T1, typename T2, typename T3, typename T4>
  void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
  {
    try
    {
      BeginMessage(pszCommand);
      ssSend << a1 << a2 << a3 << a4;
      EndMessage();
    }
    catch (...)
    {
      AbortMessage();
      throw;
    }
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5>
  void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
  {
    try
    {
      BeginMessage(pszCommand);
      ssSend << a1 << a2 << a3 << a4 << a5;
      EndMessage();
    }
    catch (...)
    {
      AbortMessage();
      throw;
    }
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6)
  {
    try
    {
      BeginMessage(pszCommand);
      ssSend << a1 << a2 << a3 << a4 << a5 << a6;
      EndMessage();
    }
    catch (...)
    {
      AbortMessage();
      throw;
    }
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
  void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7)
  {
    try
    {
      BeginMessage(pszCommand);
      ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7;
      EndMessage();
    }
    catch (...)
    {
      AbortMessage();
      throw;
    }
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
  void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8)
  {
    try
    {
      BeginMessage(pszCommand);
      ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8;
      EndMessage();
    }
    catch (...)
    {
      AbortMessage();
      throw;
    }
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
  void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8, const T9& a9)
  {
    try
    {
      BeginMessage(pszCommand);
      ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9;
      EndMessage();
    }
    catch (...)
    {
      AbortMessage();
      throw;
    }
  }


  void PushRequest(const char* pszCommand,
                   void (*fn)(void*, CDataStream&), void* param1)
  {
    uint256 hashReply;
    RAND_bytes((unsigned char*)&hashReply, sizeof(hashReply));

    {
      LOCK(cs_mapRequests);
      mapRequests[hashReply] = CRequestTracker(fn, param1);
    }

    PushMessage(pszCommand, hashReply);
  }

  template<typename T1>
  void PushRequest(const char* pszCommand, const T1& a1,
                   void (*fn)(void*, CDataStream&), void* param1)
  {
    uint256 hashReply;
    RAND_bytes((unsigned char*)&hashReply, sizeof(hashReply));

    {
      LOCK(cs_mapRequests);
      mapRequests[hashReply] = CRequestTracker(fn, param1);
    }

    PushMessage(pszCommand, hashReply, a1);
  }

  template<typename T1, typename T2>
  void PushRequest(const char* pszCommand, const T1& a1, const T2& a2,
                   void (*fn)(void*, CDataStream&), void* param1)
  {
    uint256 hashReply;
    RAND_bytes((unsigned char*)&hashReply, sizeof(hashReply));

    {
      LOCK(cs_mapRequests);
      mapRequests[hashReply] = CRequestTracker(fn, param1);
    }

    PushMessage(pszCommand, hashReply, a1, a2);
  }



  void PushGetBlocks(CBlockIndex* pindexBegin, uint256 hashEnd);
  bool IsSubscribed(unsigned int nChannel);
  void Subscribe(unsigned int nChannel, unsigned int nHops=0);
  void CancelSubscribe(unsigned int nChannel);
  void CloseSocketDisconnect();
# 706 "net.h"
  static void ClearBanned();
  static bool IsBanned(CNetAddr ip);
  bool Misbehaving(int howmuch);
  void copyStats(CNodeStats &stats);
};

inline void RelayInventory(const CInv& inv)
{

  {
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
    pnode->PushInventory(inv);
  }
}

class CTransaction;
void RelayTransaction(const CTransaction& tx, const uint256& hash);
void RelayTransaction(const CTransaction& tx, const uint256& hash, const CDataStream& ss);


