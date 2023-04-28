#include "ptrie/FileSystem.h"
#include "ptrie/Common.h"


#if defined(_WIN32)
#include <shlobj.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#endif
#include <boost/filesystem.hpp>
using namespace std;
using namespace dev;

namespace fs = boost::filesystem;

static_assert(BOOST_VERSION >= 106400, "Wrong boost headers version");


static fs::path s_dvmereumDatadir;
static fs::path s_dvmereumIpcPath;

void dev::setDataDir(fs::path const& _dataDir)
{
  s_dvmereumDatadir = _dataDir;
}

void dev::setIpcPath(fs::path const& _ipcDir)
{
  s_dvmereumIpcPath = _ipcDir;
}

fs::path dev::getIpcPath()
{

  if (s_dvmereumIpcPath.filename() == "gdvm.ipc")
  {
    return s_dvmereumIpcPath.parent_path();
  }
  else
  {
    return s_dvmereumIpcPath;
  }
}

fs::path dev::getDataDir(string _prefix)
{
  if (_prefix.empty())
  {
    _prefix = "dvmereum";
  }
  if (_prefix == "dvmereum" && !s_dvmereumDatadir.empty())
  {
    return s_dvmereumDatadir;
  }
  return getDefaultDataDir(_prefix);
}

fs::path dev::getDefaultDataDir(string _prefix)
{
  if (_prefix.empty())
  {
    _prefix = "dvmereum";
  }

#if defined(_WIN32)
  _prefix[0] = toupper(_prefix[0]);
  char path[1024] = "";
  if (SHGetSpecialFolderPathA(NULL, path, CSIDL_APPDATA, true))
  {
    return fs::path(path) / _prefix;
  }
  else
  {
#ifndef _MSC_VER

    std::cout << "getDataDir(): SHGetSpecialFolderPathA() failed.";
#endif
    BOOST_THROW_EXCEPTION(std::runtime_error("getDataDir() - SHGetSpecialFolderPathA() failed."));
  }
#else
  fs::path dataDirPath;
  char const* homeDir = getenv("HOME");
  if (!homeDir || strlen(homeDir) == 0)
  {
    struct passwd* pwd = getpwuid(getuid());
    if (pwd)
    {
      homeDir = pwd->pw_dir;
    }
  }

  if (!homeDir || strlen(homeDir) == 0)
  {
    dataDirPath = fs::path("/");
  }
  else
  {
    dataDirPath = fs::path(homeDir);
  }

  return dataDirPath / ("." + _prefix);
#endif
}

fs::path dev::appendToFilename(fs::path const& _orig, string const& _suffix)
{
  if (_orig.filename() == fs::path(".") || _orig.filename() == fs::path(".."))
  {
    return _orig / fs::path(_suffix);
  }
  else
  {
    return _orig.parent_path() / fs::path( _orig.filename().string() + _suffix);
  }
}
