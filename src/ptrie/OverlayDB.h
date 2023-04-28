#include <memory>
#include <ptrie/db.h>
#include <ptrie/Common.h>

#include <ptrie/StateCacheDB.h>

namespace dev
{

class OverlayDB: public StateCacheDB
{
public:

  explicit OverlayDB(std::unique_ptr<db::DatabaseFace> _db = nullptr)
    : m_db(_db.release(), [](db::DatabaseFace* db)
  {

    std::cout << "overlaydb " << "Closing state DB";
    delete db;
  })
  {}

  ~OverlayDB();


  OverlayDB(OverlayDB const&) = default;
  OverlayDB& operator=(OverlayDB const&) = default;

  OverlayDB(OverlayDB&&) = default;
  OverlayDB& operator=(OverlayDB&&) = default;

  void commit();
  void rollback();

  std::string lookup(h256 const& _h) const;
  bool exists(h256 const& _h) const;
  void kill(h256 const& _h);

  bytes lookupAux(h256 const& _h) const;

private:
  using StateCacheDB::clear;

  std::shared_ptr<db::DatabaseFace> m_db;
};

}
