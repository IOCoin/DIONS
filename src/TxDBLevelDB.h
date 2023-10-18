#ifndef BITCOIN_LEVELDB_H
#define BITCOIN_LEVELDB_H 

#include "ccoin/Process.h"

#include <map>
#include <string>
#include <vector>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include "BlockIndex.h"
class CTxDB
{
public:
  CTxDB(const char* pszMode="r+");
  ~CTxDB()
  {


    delete activeBatch;
  }


  void Close();

private:
  leveldb::DB *pdb;



  leveldb::WriteBatch *activeBatch;
  leveldb::Options options;
  bool fReadOnly;
  int nVersion;

protected:



  bool ScanBatch(const CDataStream &key, std::string *value, bool *deleted) const;

  template<typename K, typename T>
  bool Read(const K& key, T& value)
  {
    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(1000);
    ssKey << key;
    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {


      bool deleted = false;
      readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
      if (deleted)
      {
        return false;
      }
    }
    if (readFromDb)
    {
      leveldb::Status status = pdb->Get(leveldb::ReadOptions(),
                                        ssKey.str(), &strValue);
      if (!status.ok())
      {
        if (status.IsNotFound())
        {
          return false;
        }

        printf("LevelDB read failure: %s\n", status.ToString().c_str());
        return false;
      }
    }

    try
    {
      CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(),
                          SER_DISK, CLIENT_VERSION);
      ssValue >> value;
    }
    catch (std::exception &e)
    {
      return false;
    }
    return true;
  }

  template<typename K, typename T>
  bool Write(const K& key, const T& value)
  {
    if (fReadOnly)
    {
      assert(!"Write called on database in read-only mode");
    }

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(1000);
    ssKey << key;
    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(10000);
    ssValue << value;

    if (activeBatch)
    {
      activeBatch->Put(ssKey.str(), ssValue.str());
      return true;
    }
    leveldb::Status status = pdb->Put(leveldb::WriteOptions(), ssKey.str(), ssValue.str());
    if (!status.ok())
    {
      printf("LevelDB write failure: %s\n", status.ToString().c_str());
      return false;
    }
    return true;
  }

  template<typename K>
  bool Erase(const K& key)
  {
    if (!pdb)
    {
      return false;
    }
    if (fReadOnly)
    {
      assert(!"Erase called on database in read-only mode");
    }

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(1000);
    ssKey << key;
    if (activeBatch)
    {
      activeBatch->Delete(ssKey.str());
      return true;
    }
    leveldb::Status status = pdb->Delete(leveldb::WriteOptions(), ssKey.str());
    return (status.ok() || status.IsNotFound());
  }

  template<typename K>
  bool Exists(const K& key)
  {
    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(1000);
    ssKey << key;
    std::string unused;

    if (activeBatch)
    {
      bool deleted;
      if (ScanBatch(ssKey, &unused, &deleted) && !deleted)
      {
        return true;
      }
    }


    leveldb::Status status = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &unused);
    return status.IsNotFound() == false;
  }


public:
  bool TxnBegin();
  bool TxnCommit();
  bool TxnAbort()
  {
    delete activeBatch;
    activeBatch = NULL;
    return true;
  }

  bool ReadVersion(int& nVersion)
  {
    nVersion = 0;
    return Read(std::string("version"), nVersion);
  }

  bool WriteVersion(int nVersion)
  {
    return Write(std::string("version"), nVersion);
  }

  bool ReadTxIndex(uint256 hash, CTxIndex& txindex);
  bool UpdateTxIndex(uint256 hash, const CTxIndex& txindex);
  bool AddTxIndex(const CTransaction& tx, const CDiskTxPos& pos, int nHeight);
  bool EraseTxIndex(const CTransaction& tx);
  bool ContainsTx(uint256 hash);
  bool ReadDiskTx(uint256 hash, CTransaction& tx, CTxIndex& txindex);
  bool ReadDiskTx(uint256 hash, CTransaction& tx);
  bool ReadDiskTx(COutPoint outpoint, CTransaction& tx, CTxIndex& txindex);
  bool ReadDiskTx(COutPoint outpoint, CTransaction& tx);
  bool WriteBlockIndex(const CDiskBlockIndex& blockindex);
  bool ReadHashBestChain(uint256& hashBestChain);
  bool WriteHashBestChain(uint256 hashBestChain);
  bool ReadBestInvalidTrust(CBigNum& bnBestInvalidTrust);
  bool WriteBestInvalidTrust(CBigNum bnBestInvalidTrust);
  bool ReadSyncCheckpoint(uint256& hashCheckpoint);
  bool WriteSyncCheckpoint(uint256 hashCheckpoint);
  bool ReadCheckpointPubKey(std::string& strPubKey);
  bool WriteCheckpointPubKey(const std::string& strPubKey);
  bool LoadBlockIndex();
private:
  bool LoadBlockIndexGuts();
};


#endif
