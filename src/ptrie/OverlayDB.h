// Copyright 2014-2019 Aleth Authors.
// Copyright 2022-2023 blastdoor7.
// Licensed under the GNU General Public License, Version 3.

#pragma once

#include <memory>
#include <ptrie/db.h>
#include <ptrie/Common.h>
//XXXX #include <libdevcore/Log.h>
#include <ptrie/StateCacheDB.h>

namespace dev
{

class OverlayDB: public StateCacheDB
{
public:
    //explicit OverlayDB(std::unique_ptr<db::DatabaseFace> _db = nullptr)
    explicit OverlayDB(std::unique_ptr<db::DatabaseFace> _db = nullptr)
      : m_db(_db.release(), [](db::DatabaseFace* db) {
            //XXXX clog(VerbosityDebug, "overlaydb") << "Closing state DB";
		      std::cout << "overlaydb " << "Closing state DB";
            delete db;
        })
    {}

    ~OverlayDB();

    // Copyable
    OverlayDB(OverlayDB const&) = default;
    OverlayDB& operator=(OverlayDB const&) = default;
    // Movable
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
