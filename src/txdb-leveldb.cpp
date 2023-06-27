#include <map>

#include <boost/version.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <leveldb/env.h>
#include <leveldb/cache.h>
#include <leveldb/filter_policy.h>
#include <memenv/memenv.h>

#include "kernel.h"
#include "checkpoints.h"
#include "txdb.h"
#include "util.h"
#include "main.h"

using namespace std;
using namespace boost;

leveldb::DB *txdb;

static leveldb::Options GetOptions()
{
  leveldb::Options options;
  int nCacheSizeMB = GetArg("-dbcache", 25);
  options.block_cache = leveldb::NewLRUCache(nCacheSizeMB * 1048576);
  options.filter_policy = leveldb::NewBloomFilterPolicy(10);
  return options;
}

void init_blockindex(leveldb::Options& options, bool fRemoveOld = false)
{

  boost::filesystem::path directory = GetDataDir() / "txleveldb";

  if (fRemoveOld)
  {
    boost::filesystem::remove_all(directory);
    unsigned int nFile = 1;

    while (true)
    {
      boost::filesystem::path strBlockFile = GetDataDir() / strprintf("blk%04u.dat", nFile);


      if( !boost::filesystem::exists( strBlockFile ) )
      {
        break;
      }

      boost::filesystem::remove(strBlockFile);

      nFile++;
    }
  }

  boost::filesystem::create_directory(directory);
  printf("Opening LevelDB in %s\n", directory.string().c_str());
  leveldb::Status status = leveldb::DB::Open(options, directory.string(), &txdb);
  if (!status.ok())
  {
    throw runtime_error(strprintf("init_blockindex(): error opening database environment %s", status.ToString().c_str()));
  }
}



CTxDB::CTxDB(const char* pszMode)
{
  assert(pszMode);
  activeBatch = NULL;
  fReadOnly = (!strchr(pszMode, '+') && !strchr(pszMode, 'w'));

  if (txdb)
  {
    pdb = txdb;
    return;
  }

  bool fCreate = strchr(pszMode, 'c');

  options = GetOptions();
  options.create_if_missing = fCreate;
  options.filter_policy = leveldb::NewBloomFilterPolicy(10);

  init_blockindex(options);
  pdb = txdb;

  if (Exists(string("version")))
  {
    ReadVersion(nVersion);
    printf("Transaction index version is %d\n", nVersion);

    if (nVersion < DATABASE_VERSION)
    {
      printf("Required index version is %d, removing old database\n", DATABASE_VERSION);


      delete txdb;
      txdb = pdb = NULL;
      delete activeBatch;
      activeBatch = NULL;

      init_blockindex(options, true);
      pdb = txdb;

      bool fTmp = fReadOnly;
      fReadOnly = false;
      WriteVersion(DATABASE_VERSION);
      fReadOnly = fTmp;
    }
  }
  else if (fCreate)
  {
    bool fTmp = fReadOnly;
    fReadOnly = false;
    WriteVersion(DATABASE_VERSION);
    fReadOnly = fTmp;
  }

  printf("Opened LevelDB successfully\n");
}

void CTxDB::Close()
{
  delete txdb;
  txdb = pdb = NULL;
  delete options.filter_policy;
  options.filter_policy = NULL;
  delete options.block_cache;
  options.block_cache = NULL;
  delete activeBatch;
  activeBatch = NULL;
}

bool CTxDB::TxnBegin()
{
  assert(!activeBatch);
  activeBatch = new leveldb::WriteBatch();
  return true;
}

bool CTxDB::TxnCommit()
{
  assert(activeBatch);
  leveldb::Status status = pdb->Write(leveldb::WriteOptions(), activeBatch);
  delete activeBatch;
  activeBatch = NULL;
  if (!status.ok())
  {
    printf("LevelDB batch commit failure: %s\n", status.ToString().c_str());
    return false;
  }
  return true;
}

class CBatchScanner : public leveldb::WriteBatch::Handler
{
public:
  std::string needle;
  bool *deleted;
  std::string *foundValue;
  bool foundEntry;

  CBatchScanner() : foundEntry(false) {}

  virtual void Put(const leveldb::Slice& key, const leveldb::Slice& value)
  {
    if (key.ToString() == needle)
    {
      foundEntry = true;
      *deleted = false;
      *foundValue = value.ToString();
    }
  }

  virtual void Delete(const leveldb::Slice& key)
  {
    if (key.ToString() == needle)
    {
      foundEntry = true;
      *deleted = true;
    }
  }
};






bool CTxDB::ScanBatch(const CDataStream &key, string *value, bool *deleted) const
{
  assert(activeBatch);
  *deleted = false;
  CBatchScanner scanner;
  scanner.needle = key.str();
  scanner.deleted = deleted;
  scanner.foundValue = value;
  leveldb::Status status = activeBatch->Iterate(&scanner);
  if (!status.ok())
  {
    throw runtime_error(status.ToString());
  }
  return scanner.foundEntry;
}

bool CTxDB::ReadTxIndex(uint256 hash, CTxIndex& txindex)
{
  txindex.SetNull();
  return Read(make_pair(string("tx"), hash), txindex);
}

bool CTxDB::UpdateTxIndex(uint256 hash, const CTxIndex& txindex)
{
  return Write(make_pair(string("tx"), hash), txindex);
}

bool CTxDB::AddTxIndex(const CTransaction& tx, const CDiskTxPos& pos, int nHeight)
{

  uint256 hash = tx.GetHash();
  CTxIndex txindex(pos, tx.vout.size());
  return Write(make_pair(string("tx"), hash), txindex);
}

bool CTxDB::EraseTxIndex(const CTransaction& tx)
{
  uint256 hash = tx.GetHash();

  return Erase(make_pair(string("tx"), hash));
}

bool CTxDB::ContainsTx(uint256 hash)
{
  return Exists(make_pair(string("tx"), hash));
}

bool CTxDB::ReadDiskTx(uint256 hash, CTransaction& tx, CTxIndex& txindex)
{
  tx.SetNull();
  if (!ReadTxIndex(hash, txindex))
  {
    return false;
  }
  return (tx.ReadFromDisk(txindex.pos));
}

bool CTxDB::ReadDiskTx(uint256 hash, CTransaction& tx)
{
  CTxIndex txindex;
  return ReadDiskTx(hash, tx, txindex);
}

bool CTxDB::ReadDiskTx(COutPoint outpoint, CTransaction& tx, CTxIndex& txindex)
{
  return ReadDiskTx(outpoint.hash, tx, txindex);
}

bool CTxDB::ReadDiskTx(COutPoint outpoint, CTransaction& tx)
{
  CTxIndex txindex;
  return ReadDiskTx(outpoint.hash, tx, txindex);
}


bool CTxDB::WriteBlockIndex(const CDiskBlockIndex& blockindex)
{
  return Write(make_pair(string("blockindex"), blockindex.GetBlockHash()), blockindex);
}

bool CTxDB::ReadHashBestChain(uint256& hashBestChain)
{
  return Read(string("hashBestChain"), hashBestChain);
}

bool CTxDB::WriteHashBestChain(uint256 hashBestChain)
{
  return Write(string("hashBestChain"), hashBestChain);
}

bool CTxDB::ReadBestInvalidTrust(CBigNum& bnBestInvalidTrust)
{
  return Read(string("bnBestInvalidTrust"), bnBestInvalidTrust);
}

bool CTxDB::WriteBestInvalidTrust(CBigNum bnBestInvalidTrust)
{
  return Write(string("bnBestInvalidTrust"), bnBestInvalidTrust);
}

bool CTxDB::ReadSyncCheckpoint(uint256& hashCheckpoint)
{
  return Read(string("hashSyncCheckpoint"), hashCheckpoint);
}

bool CTxDB::WriteSyncCheckpoint(uint256 hashCheckpoint)
{
  return Write(string("hashSyncCheckpoint"), hashCheckpoint);
}

bool CTxDB::ReadCheckpointPubKey(string& strPubKey)
{
  return Read(string("strCheckpointPubKey"), strPubKey);
}

bool CTxDB::WriteCheckpointPubKey(const string& strPubKey)
{
  return Write(string("strCheckpointPubKey"), strPubKey);
}

static CBlockIndex *InsertBlockIndex(uint256 hash)
{
  if (hash == 0)
  {
    return NULL;
  }


  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
  if (mi != mapBlockIndex.end())
  {
    return (*mi).second;
  }


  CBlockIndex* pindexNew = new CBlockIndex();
  if (!pindexNew)
  {
    throw runtime_error("LoadBlockIndex() : new CBlockIndex failed");
  }
  mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
  pindexNew->phashBlock = &((*mi).first);

  return pindexNew;
}

bool CTxDB::LoadBlockIndex()
{
  if (mapBlockIndex.size() > 0)
  {


    return true;
  }



  leveldb::Iterator *iterator = pdb->NewIterator(leveldb::ReadOptions());

  CDataStream ssStartKey(SER_DISK, CLIENT_VERSION);
  ssStartKey << make_pair(string("blockindex"), uint256(0));
  iterator->Seek(ssStartKey.str());

  while (iterator->Valid())
  {

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.write(iterator->key().data(), iterator->key().size());
    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.write(iterator->value().data(), iterator->value().size());
    string strType;
    ssKey >> strType;

    if (fRequestShutdown || strType != "blockindex")
    {
      break;
    }
    CDiskBlockIndex diskindex;
    ssValue >> diskindex;

    uint256 blockHash = diskindex.GetBlockHash();


    CBlockIndex* pindexNew = InsertBlockIndex(blockHash);
    pindexNew->pprev = InsertBlockIndex(diskindex.hashPrev);
    pindexNew->pnext = InsertBlockIndex(diskindex.hashNext);
    pindexNew->nFile = diskindex.nFile;
    pindexNew->nBlockPos = diskindex.nBlockPos;
    pindexNew->nHeight = diskindex.nHeight;
    pindexNew->nMint = diskindex.nMint;
    pindexNew->nMoneySupply = diskindex.nMoneySupply;
    pindexNew->nFlags = diskindex.nFlags;
    pindexNew->nStakeModifier = diskindex.nStakeModifier;
    pindexNew->prevoutStake = diskindex.prevoutStake;
    pindexNew->nStakeTime = diskindex.nStakeTime;
    pindexNew->hashProof = diskindex.hashProof;
    pindexNew->nVersion = diskindex.nVersion;
    pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
    pindexNew->nTime = diskindex.nTime;
    pindexNew->nBits = diskindex.nBits;
    pindexNew->nNonce = diskindex.nNonce;


    if (pindexGenesisBlock == NULL && blockHash == (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet))
    {
      pindexGenesisBlock = pindexNew;
    }

    if (!pindexNew->CheckIndex())
    {
      delete iterator;
      return error("LoadBlockIndex() : CheckIndex failed at %d", pindexNew->nHeight);
    }


    if (pindexNew->IsProofOfStake())
    {
      setStakeSeen.insert(make_pair(pindexNew->prevoutStake, pindexNew->nStakeTime));
    }

    iterator->Next();
  }
  delete iterator;

  if (fRequestShutdown)
  {
    return true;
  }


  vector<pair<int, CBlockIndex*> > vSortedByHeight;
  vSortedByHeight.reserve(mapBlockIndex.size());
  BOOST_FOREACH(const PAIRTYPE(uint256, CBlockIndex*)& item, mapBlockIndex)
  {
    CBlockIndex* pindex = item.second;
    vSortedByHeight.push_back(make_pair(pindex->nHeight, pindex));
  }
  sort(vSortedByHeight.begin(), vSortedByHeight.end());
  BOOST_FOREACH(const PAIRTYPE(int, CBlockIndex*)& item, vSortedByHeight)
  {
    CBlockIndex* pindex = item.second;
    pindex->nChainTrust = (pindex->pprev ? pindex->pprev->nChainTrust : 0) + pindex->GetBlockTrust();

    pindex->nStakeModifierChecksum = GetStakeModifierChecksum(pindex);
    if (!CheckStakeModifierCheckpoints(pindex->nHeight, pindex->nStakeModifierChecksum))
    {
      return error("CTxDB::LoadBlockIndex() : Failed stake modifier checkpoint height=%d, modifier=0x%016" PRIx64, pindex->nHeight, pindex->nStakeModifier);
    }
  }


  if (!ReadHashBestChain(hashBestChain))
  {
    if (pindexGenesisBlock == NULL)
    {
      return true;
    }
    return error("CTxDB::LoadBlockIndex() : hashBestChain not loaded");
  }
  if (!mapBlockIndex.count(hashBestChain))
  {
    return error("CTxDB::LoadBlockIndex() : hashBestChain not found in the block index");
  }
  pindexBest = mapBlockIndex[hashBestChain];
  nBestHeight = pindexBest->nHeight;
  nBestChainTrust = pindexBest->nChainTrust;

  printf("LoadBlockIndex(): hashBestChain=%s  height=%d  trust=%s  date=%s\n",
         hashBestChain.ToString().substr(0,20).c_str(), nBestHeight, CBigNum(nBestChainTrust).ToString().c_str(),
         DateTimeStrFormat("%x %H:%M:%S", pindexBest->GetBlockTime()).c_str());


  if (!ReadSyncCheckpoint(Checkpoints::hashSyncCheckpoint))
  {
    return error("CTxDB::LoadBlockIndex() : hashSyncCheckpoint not loaded");
  }
  printf("LoadBlockIndex(): synchronized checkpoint %s\n", Checkpoints::hashSyncCheckpoint.ToString().c_str());


  CBigNum bnBestInvalidTrust;
  ReadBestInvalidTrust(bnBestInvalidTrust);
  nBestInvalidTrust = bnBestInvalidTrust.getuint256();


  int nCheckLevel = GetArg("-checklevel", 1);
  int nCheckDepth = GetArg( "-checkblocks", 2500);
  if (nCheckDepth == 0)
  {
    nCheckDepth = 1000000000;
  }
  if (nCheckDepth > nBestHeight)
  {
    nCheckDepth = nBestHeight;
  }
  printf("Verifying last %i blocks at level %i\n", nCheckDepth, nCheckLevel);
  CBlockIndex* pindexFork = NULL;
  map<pair<unsigned int, unsigned int>, CBlockIndex*> mapBlockPos;
  for (CBlockIndex* pindex = pindexBest; pindex && pindex->pprev; pindex = pindex->pprev)
  {
    if (fRequestShutdown || pindex->nHeight < nBestHeight-nCheckDepth)
    {
      break;
    }
    CBlock block;
    if (!block.ReadFromDisk(pindex))
    {
      return error("LoadBlockIndex() : block.ReadFromDisk failed");
    }


    if (nCheckLevel>0 && !block.CheckBlock(true, true, (nCheckLevel>6)))
    {
      printf("LoadBlockIndex() : *** found bad block at %d, hash=%s\n", pindex->nHeight, pindex->GetBlockHash().ToString().c_str());
      pindexFork = pindex->pprev;
    }

    if (nCheckLevel>1)
    {
      pair<unsigned int, unsigned int> pos = make_pair(pindex->nFile, pindex->nBlockPos);
      mapBlockPos[pos] = pindex;
      BOOST_FOREACH(const CTransaction &tx, block.vtx)
      {
        uint256 hashTx = tx.GetHash();
        CTxIndex txindex;
        if (ReadTxIndex(hashTx, txindex))
        {

          if (nCheckLevel>2 || pindex->nFile != txindex.pos.nFile || pindex->nBlockPos != txindex.pos.nBlockPos)
          {

            CTransaction txFound;
            if (!txFound.ReadFromDisk(txindex.pos))
            {
              printf("LoadBlockIndex() : *** cannot read mislocated transaction %s\n", hashTx.ToString().c_str());
              pindexFork = pindex->pprev;
            }
            else if (txFound.GetHash() != hashTx)
            {
              printf("LoadBlockIndex(): *** invalid tx position for %s\n", hashTx.ToString().c_str());
              pindexFork = pindex->pprev;
            }
          }

          unsigned int nOutput = 0;
          if (nCheckLevel>3)
          {
            BOOST_FOREACH(const CDiskTxPos &txpos, txindex.vSpent)
            {
              if (!txpos.IsNull())
              {
                pair<unsigned int, unsigned int> posFind = make_pair(txpos.nFile, txpos.nBlockPos);
                if (!mapBlockPos.count(posFind))
                {
                  printf("LoadBlockIndex(): *** found bad spend at %d, hashBlock=%s, hashTx=%s\n", pindex->nHeight, pindex->GetBlockHash().ToString().c_str(), hashTx.ToString().c_str());
                  pindexFork = pindex->pprev;
                }

                if (nCheckLevel>5)
                {
                  CTransaction txSpend;
                  if (!txSpend.ReadFromDisk(txpos))
                  {
                    printf("LoadBlockIndex(): *** cannot read spending transaction of %s:%i from disk\n", hashTx.ToString().c_str(), nOutput);
                    pindexFork = pindex->pprev;
                  }
                  else if (!txSpend.CheckTransaction())
                  {
                    printf("LoadBlockIndex(): *** spending transaction of %s:%i is invalid\n", hashTx.ToString().c_str(), nOutput);
                    pindexFork = pindex->pprev;
                  }
                  else
                  {
                    bool fFound = false;
                    BOOST_FOREACH(const CTxIn &txin, txSpend.vin)
                    if (txin.prevout.hash == hashTx && txin.prevout.n == nOutput)
                    {
                      fFound = true;
                    }
                    if (!fFound)
                    {
                      printf("LoadBlockIndex(): *** spending transaction of %s:%i does not spend it\n", hashTx.ToString().c_str(), nOutput);
                      pindexFork = pindex->pprev;
                    }
                  }
                }
              }
              nOutput++;
            }
          }
        }

        if (nCheckLevel>4)
        {
          BOOST_FOREACH(const CTxIn &txin, tx.vin)
          {
            CTxIndex txindex;
            if (ReadTxIndex(txin.prevout.hash, txindex))
              if (txindex.vSpent.size()-1 < txin.prevout.n || txindex.vSpent[txin.prevout.n].IsNull())
              {
                printf("LoadBlockIndex(): *** found unspent prevout %s:%i in %s\n", txin.prevout.hash.ToString().c_str(), txin.prevout.n, hashTx.ToString().c_str());
                pindexFork = pindex->pprev;
              }
          }
        }
      }
    }
  }
  if (pindexFork && !fRequestShutdown)
  {

    printf("LoadBlockIndex() : *** moving best chain pointer back to block %d\n", pindexFork->nHeight);
    CBlock block;
    if (!block.ReadFromDisk(pindexFork))
    {
      return error("LoadBlockIndex() : block.ReadFromDisk failed");
    }
    CTxDB txdb;
    block.SetBestChain(txdb, pindexFork);
  }

  return true;
}
