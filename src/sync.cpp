
#include "Sync.h"
#include "core/util.h"
#include <boost/foreach.hpp>
#ifdef DEBUG_LOCKCONTENTION
void PrintLockContention(const char* pszName, const char* pszFile, int nLine)
{
  printf("LOCKCONTENTION: %s\n", pszName);
  printf("Locker: %s:%d\n", pszFile, nLine);
}
#endif
#ifdef DEBUG_LOCKORDER
struct CLockLocation
{
  CLockLocation(const char* pszName, const char* pszFile, int nLine)
  {
    mutexName = pszName;
    sourceFile = pszFile;
    sourceLine = nLine;
  }
  std::string ToString() const
  {
    return mutexName+"  "+sourceFile+":"+itostr(sourceLine);
  }
  std::string MutexName() const
  {
    return mutexName;
  }
private:
  std::string mutexName;
  std::string sourceFile;
  int sourceLine;
};
typedef std::vector< std::pair<void*, CLockLocation> > LockStack;
static boost::mutex dd_mutex;
static std::map<std::pair<void*, void*>, LockStack> lockorders;
static boost::thread_specific_ptr<LockStack> lockstack;
static void potential_deadlock_detected(const std::pair<void*, void*>& mismatch, const LockStack& s1, const LockStack& s2)
{
  printf("POTENTIAL DEADLOCK DETECTED\n");
  printf("Previous lock order was:\n");
  BOOST_FOREACH(const PAIRTYPE(void*, CLockLocation)& i, s2)
  {
    if (i.first == mismatch.first)
    {
      printf(" (1)");
    }

    if (i.first == mismatch.second)
    {
      printf(" (2)");
    }

    printf(" %s\n", i.second.ToString().c_str());
  }
  printf("Current lock order is:\n");
  BOOST_FOREACH(const PAIRTYPE(void*, CLockLocation)& i, s1)
  {
    if (i.first == mismatch.first)
    {
      printf(" (1)");
    }

    if (i.first == mismatch.second)
    {
      printf(" (2)");
    }

    printf(" %s\n", i.second.ToString().c_str());
  }
}
static void push_lock(void* c, const CLockLocation& locklocation, bool fTry)
{
  if (lockstack.get() == NULL)
  {
    lockstack.reset(new LockStack);
  }

  if (fDebug)
  {
    printf("Locking: %s\n", locklocation.ToString().c_str());
  }

  dd_mutex.lock();
  (*lockstack).push_back(std::make_pair(c, locklocation));

  if (!fTry)
  {
    BOOST_FOREACH(const PAIRTYPE(void*, CLockLocation)& i, (*lockstack))
    {
      if (i.first == c)
      {
        break;
      }

      std::pair<void*, void*> p1 = std::make_pair(i.first, c);

      if (lockorders.count(p1))
      {
        continue;
      }

      lockorders[p1] = (*lockstack);
      std::pair<void*, void*> p2 = std::make_pair(c, i.first);

      if (lockorders.count(p2))
      {
        potential_deadlock_detected(p1, lockorders[p2], lockorders[p1]);
        break;
      }
    }
  }

  dd_mutex.unlock();
}
static void pop_lock()
{
  if (fDebug)
  {
    const CLockLocation& locklocation = (*lockstack).rbegin()->second;
    printf("Unlocked: %s\n", locklocation.ToString().c_str());
  }

  dd_mutex.lock();
  (*lockstack).pop_back();
  dd_mutex.unlock();
}
void EnterCritical(const char* pszName, const char* pszFile, int nLine, void* cs, bool fTry)
{
  push_lock(cs, CLockLocation(pszName, pszFile, nLine), fTry);
}
void LeaveCritical()
{
  pop_lock();
}
std::string LocksHeld()
{
  std::string result;
  BOOST_FOREACH(const PAIRTYPE(void*, CLockLocation)&i, *lockstack)
  result += i.second.ToString() + std::string("\n");
  return result;
}
void AssertLockHeldInternal(const char *pszName, const char* pszFile, int nLine, void *cs)
{
  BOOST_FOREACH(const PAIRTYPE(void*, CLockLocation)&i, *lockstack)

  if (i.first == cs)
  {
    return;
  }

  fprintf(stderr, "Assertion failed: lock %s not held in %s:%i; locks held:\n%s",
          pszName, pszFile, nLine, LocksHeld().c_str());
  abort();
}
#endif
