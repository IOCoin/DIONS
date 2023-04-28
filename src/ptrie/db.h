#include "ptrie/Exceptions.h"
#include "ptrie/dbfwd.h"

#include <memory>
#include <string>

namespace dev
{
namespace db
{


class WriteBatchFace
{
public:
  virtual ~WriteBatchFace() = default;

  virtual void insert(Slice _key, Slice _value) = 0;
  virtual void kill(Slice _key) = 0;

protected:
  WriteBatchFace() = default;

  WriteBatchFace(WriteBatchFace const&) = delete;
  WriteBatchFace& operator=(WriteBatchFace const&) = delete;

  WriteBatchFace(WriteBatchFace&&) = delete;
  WriteBatchFace& operator=(WriteBatchFace&&) = delete;
};

class DatabaseFace
{
public:
  virtual ~DatabaseFace() = default;
  virtual std::string lookup(Slice _key) const = 0;
  virtual bool exists(Slice _key) const = 0;
  virtual void insert(Slice _key, Slice _value) = 0;
  virtual void kill(Slice _key) = 0;

  virtual std::unique_ptr<WriteBatchFace> createWriteBatch() const = 0;
  virtual void commit(std::unique_ptr<WriteBatchFace> _batch) = 0;





  virtual void forEach(std::function<bool(Slice, Slice)> f) const = 0;
};

DEV_SIMPLE_EXCEPTION(DatabaseError);

enum class DatabaseStatus
{
  Ok,
  NotFound,
  Corruption,
  NotSupported,
  InvalidArgument,
  IOError,
  Unknown
};

using errinfo_dbStatusCode = boost::error_info<struct tag_dbStatusCode, DatabaseStatus>;
using errinfo_dbStatusString = boost::error_info<struct tag_dbStatusString, std::string>;

}
}
