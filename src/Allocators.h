#pragma once

#include <string.h>
#include <string>
#include <boost/thread/mutex.hpp>
#include <map>

#ifdef WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN 1
#ifndef NOMINMAX
#define NOMINMAX 
#endif
#include <windows.h>




#else
#include <sys/mman.h>
#include <limits.h>
#include <unistd.h>
#endif
# 44 "allocators.h"
template <class Locker> class LockedPageManagerBase
{
public:
  LockedPageManagerBase(size_t page_size):
    page_size(page_size)
  {

    assert(!(page_size & (page_size-1)));
    page_mask = ~(page_size - 1);
  }


  void LockRange(void *p, size_t size)
  {
    boost::mutex::scoped_lock lock(mutex);
    if(!size)
    {
      return;
    }
    const size_t base_addr = reinterpret_cast<size_t>(p);
    const size_t start_page = base_addr & page_mask;
    const size_t end_page = (base_addr + size - 1) & page_mask;
    for(size_t page = start_page; page <= end_page; page += page_size)
    {
      Histogram::iterator it = histogram.find(page);
      if(it == histogram.end())
      {
        locker.Lock(reinterpret_cast<void*>(page), page_size);
        histogram.insert(std::make_pair(page, 1));
      }
      else
      {
        it->second += 1;
      }
    }
  }


  void UnlockRange(void *p, size_t size)
  {
    boost::mutex::scoped_lock lock(mutex);
    if(!size)
    {
      return;
    }
    const size_t base_addr = reinterpret_cast<size_t>(p);
    const size_t start_page = base_addr & page_mask;
    const size_t end_page = (base_addr + size - 1) & page_mask;
    for(size_t page = start_page; page <= end_page; page += page_size)
    {
      Histogram::iterator it = histogram.find(page);
      assert(it != histogram.end());

      it->second -= 1;
      if(it->second == 0)
      {

        locker.Unlock(reinterpret_cast<void*>(page), page_size);
        histogram.erase(it);
      }
    }
  }


  int GetLockedPageCount()
  {
    boost::mutex::scoped_lock lock(mutex);
    return histogram.size();
  }

private:
  Locker locker;
  boost::mutex mutex;
  size_t page_size, page_mask;

  typedef std::map<size_t,int> Histogram;
  Histogram histogram;
};


static inline size_t GetSystemPageSize()
{
  size_t page_size;
#if defined(WIN32)
  SYSTEM_INFO sSysInfo;
  GetSystemInfo(&sSysInfo);
  page_size = sSysInfo.dwPageSize;
#elif defined(PAGESIZE)
  page_size = PAGESIZE;
#else
  page_size = sysconf(_SC_PAGESIZE);
#endif
  return page_size;
}





class MemoryPageLocker
{
public:



  bool Lock(const void *addr, size_t len)
  {
#ifdef WIN32
    return VirtualLock(const_cast<void*>(addr), len);
#else
    return mlock(addr, len) == 0;
#endif
  }



  bool Unlock(const void *addr, size_t len)
  {
#ifdef WIN32
    return VirtualUnlock(const_cast<void*>(addr), len);
#else
    return munlock(addr, len) == 0;
#endif
  }
};





class LockedPageManager: public LockedPageManagerBase<MemoryPageLocker>
{
public:
  static LockedPageManager instance;
private:
  LockedPageManager():
    LockedPageManagerBase<MemoryPageLocker>(GetSystemPageSize())
  {}
};





template<typename T>
struct secure_allocator : public std::allocator<T>
{

  typedef std::allocator<T> base;
  typedef typename base::size_type size_type;
  typedef typename base::difference_type difference_type;
  typedef typename base::pointer pointer;
  typedef typename base::const_pointer const_pointer;
  typedef typename base::reference reference;
  typedef typename base::const_reference const_reference;
  typedef typename base::value_type value_type;
  secure_allocator() throw() {}
  secure_allocator(const secure_allocator& a) throw() : base(a) {}
  template <typename U>
  secure_allocator(const secure_allocator<U>& a) throw() : base(a) {}
  ~secure_allocator() throw() {}
  template<typename _Other> struct rebind
  {
    typedef secure_allocator<_Other> other;
  };

  T* allocate(std::size_t n, const void *hint = 0)
  {
    T *p;
    p = std::allocator<T>::allocate(n, hint);
    if (p != NULL)
    {
      LockedPageManager::instance.LockRange(p, sizeof(T) * n);
    }
    return p;
  }

  void deallocate(T* p, std::size_t n)
  {
    if (p != NULL)
    {
      memset(p, 0, sizeof(T) * n);
      LockedPageManager::instance.UnlockRange(p, sizeof(T) * n);
    }
    std::allocator<T>::deallocate(p, n);
  }
};





template<typename T>
struct zero_after_free_allocator : public std::allocator<T>
{

  typedef std::allocator<T> base;
  typedef typename base::size_type size_type;
  typedef typename base::difference_type difference_type;
  typedef typename base::pointer pointer;
  typedef typename base::const_pointer const_pointer;
  typedef typename base::reference reference;
  typedef typename base::const_reference const_reference;
  typedef typename base::value_type value_type;
  zero_after_free_allocator() throw() {}
  zero_after_free_allocator(const zero_after_free_allocator& a) throw() : base(a) {}
  template <typename U>
  zero_after_free_allocator(const zero_after_free_allocator<U>& a) throw() : base(a) {}
  ~zero_after_free_allocator() throw() {}
  template<typename _Other> struct rebind
  {
    typedef zero_after_free_allocator<_Other> other;
  };

  void deallocate(T* p, std::size_t n)
  {
    if (p != NULL)
    {
      memset(p, 0, sizeof(T) * n);
    }
    std::allocator<T>::deallocate(p, n);
  }
};


typedef std::basic_string<char, std::char_traits<char>, secure_allocator<char> > SecureString;

