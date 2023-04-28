#include <string>
#include <boost/filesystem.hpp>

namespace dev
{


void setDataDir(boost::filesystem::path const& _dir);

boost::filesystem::path getDataDir(std::string _prefix = "dvmereum");

boost::filesystem::path getDefaultDataDir(std::string _prefix = "dvmereum");

void setIpcPath(boost::filesystem::path const& _ipcPath);

boost::filesystem::path getIpcPath();


boost::filesystem::path appendToFilename(boost::filesystem::path const& _orig, std::string const& _suffix);

}
