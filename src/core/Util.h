#ifndef BITCOIN_UTIL_H
#define BITCOIN_UTIL_H 

#include "uint256.h"

#ifndef WIN32
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <map>
#include <vector>
#include <string>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <openssl/sha.h>
#include <openssl/ripemd.h>

#include "NetBase.h"


#define __STDC_FORMAT_MACROS 1

#include <stdint.h>
#include <inttypes.h>

#include "Constants.h"
#include "Relay.h"

static const int64_t COIN = 100000000;
static const int64_t CENT = 1000000;

#define BEGIN(a) ((char*)&(a))
#define END(a) ((char*)&((&(a))[1]))
#define UBEGIN(a) ((unsigned char*)&(a))
#define UEND(a) ((unsigned char*)&((&(a))[1]))
#define ARRAYLEN(array) (sizeof(array)/sizeof((array)[0]))

#define UVOIDBEGIN(a) ((void*)&(a))
#define CVOIDBEGIN(a) ((const void*)&(a))
#define UINTBEGIN(a) ((uint32_t*)&(a))
#define CUINTBEGIN(a) ((const uint32_t*)&(a))


#if defined(_MSC_VER) || defined(__MSVCRT__)



#define PRIszx "Ix"
#define PRIszu "Iu"
#define PRIszd "Id"
#define PRIpdx "Ix"
#define PRIpdu "Iu"
#define PRIpdd "Id"
#else
#define PRIszx "zx"
#define PRIszu "zu"
#define PRIszd "zd"
#define PRIpdx "tx"
#define PRIpdu "tu"
#define PRIpdd "td"
#endif


#define PAIRTYPE(t1,t2) std::pair<t1, t2>


template <size_t nBytes, typename T>
T* alignup(T* p)
{
  union
  {
    T* ptr;
    size_t n;
  } u;
  u.ptr = p;
  u.n = (u.n + (nBytes-1)) & ~(nBytes-1);
  return u.ptr;
}

#ifdef WIN32
#define MSG_NOSIGNAL 0
#define MSG_DONTWAIT 0

#ifndef S_IRUSR
#define S_IRUSR 0400
#define S_IWUSR 0200
#endif
#else
#define MAX_PATH 1024
#endif

inline void MilliSleep(int64_t n)
{
#if BOOST_VERSION >= 105000
  boost::this_thread::sleep_for(boost::chrono::milliseconds(n));
#else
  boost::this_thread::sleep(boost::posix_time::milliseconds(n));
#endif
}





#ifdef __GNUC__
#define ATTR_WARN_PRINTF(X,Y) __attribute__((format(printf,X,Y)))
#else
#define ATTR_WARN_PRINTF(X,Y) 
#endif
extern std::map<std::string, std::string> mapArgs;
extern std::map<std::string, std::vector<std::string> > mapMultiArgs;
extern bool fDebug;
extern bool fDebugNet;
extern bool fPrintToConsole;
extern bool fPrintToDebugger;
extern bool fRequestShutdown;
extern bool fShutdown;
extern bool fDaemon;
extern bool fServer;
extern bool fCommandLine;
extern std::string strMiscWarning;
extern bool fTestNet;
extern bool fViewWallet;
extern bool fNoListen;
extern bool fLogTimestamps;
extern bool fReopenDebugLog;

bool EncryptMessageAES(const std::string& message, std::string& encryptedMsg, std::vector<unsigned char>& key, std::string& iv128Base64);
bool DecryptMessageAES(const std::string& encryptedMsg, std::string& message, std::vector<unsigned char>& key, std::string& iv128Base64);
bool EncryptMessage(const std::string& rsaPubKey, const std::string& message, std::string& encryptedMsg);
bool DecryptMessage(const std::string& rsaPrivKey, const std::string& encrpyted, std::string& decryptedMsg);
void GenerateRSAKey(CoordinateVector& p);
void GenerateAESKey(vchType& rsaPubKey);

int fqa__7(vector<unsigned char>&);

void RandAddSeed();
void RandAddSeedPerfmon();
int ATTR_WARN_PRINTF(1,2) OutputDebugStringF(const char* pszFormat, ...);
# 169 "util.h"
std::string ATTR_WARN_PRINTF(1,3) real_strprintf(const char *format, int dummy, ...);



std::string real_strprintf(const std::string &format, int dummy, ...);
#define strprintf(format,...) real_strprintf(format, 0, __VA_ARGS__)
std::string vstrprintf(const char *format, va_list ap);

bool ATTR_WARN_PRINTF(1,2) error(const char *format, ...);







#define printf OutputDebugStringF

void PrintException(std::exception* pex, const char* pszThread);
void PrintExceptionContinue(std::exception* pex, const char* pszThread);
void ParseString(const std::string& str, char c, std::vector<std::string>& v);
std::string FormatMoney(int64_t n, bool fPlus=false);
bool ParseMoney(const std::string& str, int64_t& nRet);
bool ParseMoney(const char* pszIn, int64_t& nRet);
std::vector<unsigned char> ParseHex(const char* psz);
std::vector<unsigned char> ParseHex(const std::string& str);
bool IsHex(const std::string& str);
std::vector<unsigned char> DecodeBase64(const char* p, bool* pfInvalid = NULL);
std::string DecodeBase64(const std::string& str);
std::string EncodeBase64(const unsigned char* pch, size_t len);
std::string EncodeBase64(const std::string& str);
std::vector<unsigned char> DecodeBase32(const char* p, bool* pfInvalid = NULL);
std::string DecodeBase32(const std::string& str);
std::string EncodeBase32(const unsigned char* pch, size_t len);
std::string EncodeBase32(const std::string& str);
void ParseParameters(int argc, const char*const argv[]);
bool WildcardMatch(const char* psz, const char* mask);
bool WildcardMatch(const std::string& str, const std::string& mask);
void FileCommit(FILE *fileout);
bool RenameOver(boost::filesystem::path src, boost::filesystem::path dest);
boost::filesystem::path GetDefaultDataDir();
const boost::filesystem::path &GetDataDir(bool fNetSpecific = true);
boost::filesystem::path GetConfigFile();
boost::filesystem::path GetPidFile();
#ifndef WIN32
void CreatePidFile(const boost::filesystem::path &path, pid_t pid);
#endif
void ReadConfigFile(std::map<std::string, std::string>& mapSettingsRet, std::map<std::string, std::vector<std::string> >& mapMultiSettingsRet);
#ifdef WIN32
boost::filesystem::path GetSpecialFolderPath(int nFolder, bool fCreate = true);
#endif
void ShrinkDebugFile();
int GetRandInt(int nMax);
uint64_t GetRand(uint64_t nMax);
uint256 GetRandHash();
int64_t GetTime();
void SetMockTime(int64_t nMockTimeIn);
int64_t GetAdjustedTime();
int64_t GetTimeOffset();
std::string FormatFullVersion();
std::string FormatSubVersion(const std::string& name, int nClientVersion, const std::vector<std::string>& comments);
void AddTimeData(const CNetAddr& ip, int64_t nTime);
void runCommand(std::string strCommand);
double nBitsToDifficulty(unsigned int nBits);
# 241 "util.h"
inline std::string i64tostr(int64_t n)
{
  return strprintf("%" PRId64, n);
}

inline std::string itostr(int n)
{
  return strprintf("%d", n);
}

inline int64_t atoi64(const char* psz)
{
#ifdef _MSC_VER
  return _atoi64(psz);
#else
  return strtoll(psz, NULL, 10);
#endif
}

inline int64_t atoi64(const std::string& str)
{
#ifdef _MSC_VER
  return _atoi64(str.c_str());
#else
  return strtoll(str.c_str(), NULL, 10);
#endif
}

inline int atoi(const std::string& str)
{
  return atoi(str.c_str());
}

inline int roundint(double d)
{
  return (int)(d > 0 ? d + 0.5 : d - 0.5);
}

inline int64_t roundint64(double d)
{
  return (int64_t)(d > 0 ? d + 0.5 : d - 0.5);
}

inline int64_t abs64(int64_t n)
{
  return (n >= 0 ? n : -n);
}

inline std::string leftTrim(std::string src, char chr)
{
  std::string::size_type pos = src.find_first_not_of(chr, 0);

  if(pos > 0)
  {
    src.erase(0, pos);
  }

  return src;
}

template<typename T>
std::string HexStr(const T itbegin, const T itend, bool fSpaces=false)
{
  std::string rv;
  static const char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                   '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
                                 };
  rv.reserve((itend-itbegin)*3);
  for(T it = itbegin; it < itend; ++it)
  {
    unsigned char val = (unsigned char)(*it);
    if(fSpaces && it != itbegin)
    {
      rv.push_back(' ');
    }
    rv.push_back(hexmap[val>>4]);
    rv.push_back(hexmap[val&15]);
  }

  return rv;
}

inline std::string HexStr(const std::vector<unsigned char>& vch, bool fSpaces=false)
{
  return HexStr(vch.begin(), vch.end(), fSpaces);
}

inline int64_t GetPerformanceCounter()
{
  int64_t nCounter = 0;
#ifdef WIN32
  QueryPerformanceCounter((LARGE_INTEGER*)&nCounter);
#else
  timeval t;
  gettimeofday(&t, NULL);
  nCounter = (int64_t) t.tv_sec * 1000000 + t.tv_usec;
#endif
  return nCounter;
}

inline int64_t GetTimeMillis()
{
  return (boost::posix_time::ptime(boost::posix_time::microsec_clock::universal_time()) -
          boost::posix_time::ptime(boost::gregorian::date(1970,1,1))).total_milliseconds();
}

inline std::string DateTimeStrFormat(const char* pszFormat, int64_t nTime)
{
  time_t n = nTime;
  struct tm* ptmTime = gmtime(&n);
  char pszTime[200];
  strftime(pszTime, sizeof(pszTime), pszFormat, ptmTime);
  return pszTime;
}

static const std::string strTimestampFormat = "%Y-%m-%d %H:%M:%S UTC";
inline std::string DateTimeStrFormat(int64_t nTime)
{
  return DateTimeStrFormat(strTimestampFormat.c_str(), nTime);
}


template<typename T>
void skipspaces(T& it)
{
  while (isspace(*it))
  {
    ++it;
  }
}

inline bool IsSwitchChar(char c)
{
#ifdef WIN32
  return c == '-' || c == '/';
#else
  return c == '-';
#endif
}
# 388 "util.h"
std::string GetArg(const std::string& strArg, const std::string& strDefault);
# 397 "util.h"
int64_t GetArg(const std::string& strArg, int64_t nDefault);
# 406 "util.h"
bool GetBoolArg(const std::string& strArg, bool fDefault=false);
# 415 "util.h"
bool SoftSetArg(const std::string& strArg, const std::string& strValue);
# 424 "util.h"
bool SoftSetBoolArg(const std::string& strArg, bool fValue);
# 433 "util.h"
extern uint32_t insecure_rand_Rz;
extern uint32_t insecure_rand_Rw;
static inline uint32_t insecure_rand(void)
{
  insecure_rand_Rz=36969*(insecure_rand_Rz&65535)+(insecure_rand_Rz>>16);
  insecure_rand_Rw=18000*(insecure_rand_Rw&65535)+(insecure_rand_Rw>>16);
  return (insecure_rand_Rw<<16)+insecure_rand_Rz;
}





void seed_insecure_rand(bool fDeterministic=false);

template<typename T1>
inline uint256 Hash(const T1 pbegin, const T1 pend)
{
  static unsigned char pblank[1];
  uint256 hash1;
  SHA256((pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]), (unsigned char*)&hash1);
  uint256 hash2;
  SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
  return hash2;
}

class CHashWriter
{
private:
  SHA256_CTX ctx;

public:
  int nType;
  int nVersion;

  void Init()
  {
    SHA256_Init(&ctx);
  }

  CHashWriter(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn)
  {
    Init();
  }

  CHashWriter& write(const char *pch, size_t size)
  {
    SHA256_Update(&ctx, pch, size);
    return (*this);
  }


  uint256 GetHash()
  {
    uint256 hash1;
    SHA256_Final((unsigned char*)&hash1, &ctx);
    uint256 hash2;
    SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
  }

  template<typename T>
  CHashWriter& operator<<(const T& obj)
  {

    ::Serialize(*this, obj, nType, nVersion);
    return (*this);
  }
};


template<typename T1, typename T2>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end)
{
  static unsigned char pblank[1];
  uint256 hash1;
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
  SHA256_Update(&ctx, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
  SHA256_Final((unsigned char*)&hash1, &ctx);
  uint256 hash2;
  SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
  return hash2;
}

template<typename T1, typename T2, typename T3>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end)
{
  static unsigned char pblank[1];
  uint256 hash1;
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
  SHA256_Update(&ctx, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
  SHA256_Update(&ctx, (p3begin == p3end ? pblank : (unsigned char*)&p3begin[0]), (p3end - p3begin) * sizeof(p3begin[0]));
  SHA256_Final((unsigned char*)&hash1, &ctx);
  uint256 hash2;
  SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
  return hash2;
}

template<typename T>
uint256 SerializeHash(const T& obj, int nType=SER_GETHASH, int nVersion=PROTOCOL_VERSION)
{
  CHashWriter ss(nType, nVersion);
  ss << obj;
  return ss.GetHash();
}

inline uint160 Hash160(const std::vector<unsigned char>& vch)
{
  uint256 hash1;
  SHA256(&vch[0], vch.size(), (unsigned char*)&hash1);
  uint160 hash2;
  RIPEMD160((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
  return hash2;
}






template <typename T>
bool TimingResistantEqual(const T& a, const T& b)
{
  if (b.size() == 0)
  {
    return a.size() == 0;
  }
  size_t accumulator = a.size() ^ b.size();
  for (size_t i = 0; i < a.size(); i++)
  {
    accumulator |= a[i] ^ b[i%b.size()];
  }
  return accumulator == 0;
}




template <typename T> class CMedianFilter
{
private:
  std::vector<T> vValues;
  std::vector<T> vSorted;
  unsigned int nSize;
public:
  CMedianFilter(unsigned int size, T initial_value):
    nSize(size)
  {
    vValues.reserve(size);
    vValues.push_back(initial_value);
    vSorted = vValues;
  }

  void input(T value)
  {
    if(vValues.size() == nSize)
    {
      vValues.erase(vValues.begin());
    }
    vValues.push_back(value);

    vSorted.resize(vValues.size());
    std::copy(vValues.begin(), vValues.end(), vSorted.begin());
    std::sort(vSorted.begin(), vSorted.end());
  }

  T median() const
  {
    int size = vSorted.size();
    assert(size>0);
    if(size & 1)
    {
      return vSorted[size/2];
    }
    else
    {
      return (vSorted[size/2-1] + vSorted[size/2]) / 2;
    }
  }

  int size() const
  {
    return vValues.size();
  }

  std::vector<T> sorted () const
  {
    return vSorted;
  }
};

bool NewThread(void(*pfn)(void*), void* parg);

#ifdef WIN32
inline void SetThreadPriority(int nPriority)
{
  SetThreadPriority(GetCurrentThread(), nPriority);
}
#else

#define THREAD_PRIORITY_LOWEST PRIO_MAX
#define THREAD_PRIORITY_BELOW_NORMAL 2
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_ABOVE_NORMAL 0

inline void SetThreadPriority(int nPriority)
{


#ifdef PRIO_THREAD
  setpriority(PRIO_THREAD, 0, nPriority);
#else
  setpriority(PRIO_PROCESS, 0, nPriority);
#endif
}

inline void ExitThread(size_t nExitCode)
{
  pthread_exit((void*)nExitCode);
}
#endif

void RenameThread(const char* name);

inline std::string stripSpacesAndQuotes(std::string s)
{
  const char *pcc_Char2Cut = " '\"";
  s.erase(s.find_last_not_of(pcc_Char2Cut)+1);
  s.erase( 0, s.find_first_not_of(pcc_Char2Cut));
  return s;
}

inline bool isOnlyWhiteSpace(const std::string& str)
{
  bool e1 = str.find_first_not_of (' ') == str.npos;
  bool e2 = str == "";
  bool e3 = str.size() == 0;

  return (e1 || e2 || e3);
}

inline uint32_t ByteReverse(uint32_t value)
{
  value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
  return (value<<16) | (value>>16);
}

int atod(const std::string&, std::string&);

#endif
