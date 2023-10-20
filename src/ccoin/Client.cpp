#include "Client.h"

using namespace std;
using namespace boost;

bool Client::init(boost::filesystem::path const& _dbPath)
{
  try
  {
    printf("Opening state database...\n");
    std::unique_ptr<dev::db::DatabaseFace> db = dev::db::DBFactory::create(_dbPath / "state");
    this->overlayDB_ = dev::OverlayDB(std::move(db));
  }
  catch (boost::exception const& ex)
  {
    if (dev::db::isDiskDatabase())
    {
      printf("Error opening state database\n");
      dev::db::DatabaseStatus const dbStatus =
        *boost::get_error_info<dev::db::errinfo_dbStatusCode>(ex);

      if (boost::filesystem::space(GetDataDir() / "state").available < 1024)
      {
        return this->uiFace_->initError_("Insufficient disk space : "
                         "Not enough available space on hard drive."
                         "Please back up disk first - free up some space and then re run. Exiting.");
      }
      else if (dbStatus == dev::db::DatabaseStatus::Corruption)
      {
        printf("Database corruption detected. Please see the exception for corruption "
               "details. Exception: %s\n", boost::diagnostic_information(ex).c_str());
        throw runtime_error("Database corruption");
        string msg = strprintf(_(
                                 " Database corruption detected. Details : Exception %s\n"
                               ), boost::diagnostic_information(ex));
        return this->uiFace_->initError_(msg);
      }
      else if (dbStatus == dev::db::DatabaseStatus::IOError)
      {
        return this->uiFace_->initError_("Database already open. You appear to have "
                         "another instance running on the same data path.");
      }
    }

    string msg = strprintf(_(
                             "statedb: Unknown error encountered when opening state database. Details : Exception %s\n"
                           ), boost::diagnostic_information(ex));
    return this->uiFace_->initError_(msg);
  }

  return true;
}
