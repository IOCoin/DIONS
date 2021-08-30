// Licensed under the GNU General Public License, Version 3.

#pragma once

#include <string>
#include <boost/filesystem.hpp>

namespace dev
{

/// Sets the data dir for the default ("dvmereum") prefix.
void setDataDir(boost::filesystem::path const& _dir);
/// @returns the path for user data.
boost::filesystem::path getDataDir(std::string _prefix = "dvmereum");
/// @returns the default path for user data, ignoring the one set by `setDataDir`.
boost::filesystem::path getDefaultDataDir(std::string _prefix = "dvmereum");
/// Sets the ipc socket dir
void setIpcPath(boost::filesystem::path const& _ipcPath);
/// @returns the ipc path (default is DataDir)
boost::filesystem::path getIpcPath();

/// @returns a new path whose file name is suffixed with the given suffix.
boost::filesystem::path appendToFilename(boost::filesystem::path const& _orig, std::string const& _suffix);

}
