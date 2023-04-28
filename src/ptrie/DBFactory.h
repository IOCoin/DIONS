#include "Common.h"
#include "ptrie/db.h"

#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>

namespace dev
{
namespace db
{
enum class DatabaseKind
{
  LevelDB
};





boost::program_options::options_description databaseProgramOptions(
  unsigned _lineLength = boost::program_options::options_description::m_default_line_length);

bool isDiskDatabase();
DatabaseKind databaseKind();
void setDatabaseKindByName(std::string const& _name);
void setDatabaseKind(DatabaseKind _kind);
boost::filesystem::path databasePath();

class DBFactory
{
public:
  DBFactory() = delete;
  ~DBFactory() = delete;

  static std::unique_ptr<DatabaseFace> create();
  static std::unique_ptr<DatabaseFace> create(boost::filesystem::path const& _path);
  static std::unique_ptr<DatabaseFace> create(DatabaseKind _kind);
  static std::unique_ptr<DatabaseFace> create(
    DatabaseKind _kind, boost::filesystem::path const& _path);

private:
};
}
}
