#include "db.h"
#include "net.h"
#include "util.h"
#include "main.h"
#include "ui_interface.h"
#include "base58.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#ifndef WIN32
#include "sys/stat.h"
#endif

using namespace std;
using namespace boost;


unsigned int nWalletDBUpdated;

extern unsigned int scaleMonitor();





CDBEnv bitdb;

void CDBEnv::EnvShutdown()
{
  if (!fDbEnvInit)
  {
    return;
  }

  fDbEnvInit = false;
  int ret = dbenv.close(0);
  if (ret != 0)
  {
    printf("EnvShutdown exception: %s (%d)\n", DbEnv::strerror(ret), ret);
  }
  if (!fMockDb)
  {
    DbEnv((u_int32_t)0).remove(strPath.c_str(), 0);
  }
}

CDBEnv::CDBEnv() : dbenv(DB_CXX_NO_EXCEPTIONS)
{
  fDbEnvInit = false;
  fMockDb = false;
}

CDBEnv::~CDBEnv()
{
  EnvShutdown();
}

void CDBEnv::Close()
{
  EnvShutdown();
}

bool CDBEnv::Open(boost::filesystem::path pathEnv_)
{
  if (fDbEnvInit)
  {
    return true;
  }

  if (fShutdown)
  {
    return false;
  }

  pathEnv = pathEnv_;
  boost::filesystem::path pathDataDir = pathEnv;
  strPath = pathDataDir.string();
  boost::filesystem::path pathLogDir = pathDataDir / "database";
  boost::filesystem::create_directory(pathLogDir);
  boost::filesystem::path pathErrorFile = pathDataDir / "db.log";
  printf("dbenv.open LogDir=%s ErrorFile=%s\n", pathLogDir.string().c_str(), pathErrorFile.string().c_str());

  unsigned int nEnvFlags = 0;
  if (GetBoolArg("-privdb", true))
  {
    nEnvFlags |= DB_PRIVATE;
  }

  int nDbCache = GetArg("-dbcache", 25);
  dbenv.set_lg_dir(pathLogDir.string().c_str());
  dbenv.set_cachesize(nDbCache / 1024, (nDbCache % 1024)*1048576, 1);
  dbenv.set_lg_bsize(1048576);
  dbenv.set_lg_max(10485760);



  dbenv.set_lk_max_locks(537000);

  dbenv.set_lk_max_objects(10000);
  dbenv.set_errfile(fopen(pathErrorFile.string().c_str(), "a"));
  dbenv.set_flags(DB_AUTO_COMMIT, 1);
  dbenv.set_flags(DB_TXN_WRITE_NOSYNC, 1);
#ifdef DB_LOG_AUTO_REMOVE
  dbenv.log_set_config(DB_LOG_AUTO_REMOVE, 1);
#endif
  int ret = dbenv.open(strPath.c_str(),
                       DB_CREATE |
                       DB_INIT_LOCK |
                       DB_INIT_LOG |
                       DB_INIT_MPOOL |
                       DB_INIT_TXN |
                       DB_THREAD |
                       DB_RECOVER |
                       nEnvFlags,
                       S_IRUSR | S_IWUSR);
  if (ret != 0)
  {
    return error("CDB() : error %s (%d) opening database environment", DbEnv::strerror(ret), ret);
  }

  fDbEnvInit = true;
  fMockDb = false;

  return true;
}

void CDBEnv::MakeMock()
{
  if (fDbEnvInit)
  {
    throw runtime_error("CDBEnv::MakeMock(): already initialized");
  }

  if (fShutdown)
  {
    throw runtime_error("CDBEnv::MakeMock(): during shutdown");
  }

  printf("CDBEnv::MakeMock()\n");

  dbenv.set_cachesize(1, 0, 1);
  dbenv.set_lg_bsize(10485760*4);
  dbenv.set_lg_max(10485760);
  dbenv.set_lk_max_locks(10000);
  dbenv.set_lk_max_objects(10000);
  dbenv.set_flags(DB_AUTO_COMMIT, 1);
#ifdef DB_LOG_IN_MEMORY
  dbenv.log_set_config(DB_LOG_IN_MEMORY, 1);
#endif
  int ret = dbenv.open(NULL,
                       DB_CREATE |
                       DB_INIT_LOCK |
                       DB_INIT_LOG |
                       DB_INIT_MPOOL |
                       DB_INIT_TXN |
                       DB_THREAD |
                       DB_PRIVATE,
                       S_IRUSR | S_IWUSR);
  if (ret > 0)
  {
    throw runtime_error(strprintf("CDBEnv::MakeMock(): error %d opening database environment", ret));
  }

  fDbEnvInit = true;
  fMockDb = true;
}

CDBEnv::VerifyResult CDBEnv::Verify(std::string strFile, bool (*recoverFunc)(CDBEnv& dbenv, std::string strFile))
{
  LOCK(cs_db);
  assert(mapFileUseCount.count(strFile) == 0);

  Db db(&dbenv, 0);
  int result = db.verify(strFile.c_str(), NULL, NULL, 0);
  if (result == 0)
  {
    return VERIFY_OK;
  }
  else if (recoverFunc == NULL)
  {
    return RECOVER_FAIL;
  }


  bool fRecovered = (*recoverFunc)(*this, strFile);
  return (fRecovered ? RECOVER_OK : RECOVER_FAIL);
}

bool CDBEnv::Salvage(std::string strFile, bool fAggressive,
                     std::vector<CDBEnv::KeyValPair >& vResult)
{
  LOCK(cs_db);
  assert(mapFileUseCount.count(strFile) == 0);

  u_int32_t flags = DB_SALVAGE;
  if (fAggressive)
  {
    flags |= DB_AGGRESSIVE;
  }

  stringstream strDump;

  Db db(&dbenv, 0);
  int result = db.verify(strFile.c_str(), NULL, &strDump, flags);
  if (result == DB_VERIFY_BAD)
  {
    printf("Error: Salvage found errors, all data may not be recoverable.\n");
    if (!fAggressive)
    {
      printf("Error: Rerun with aggressive mode to ignore errors and continue.\n");
      return false;
    }
  }
  if (result != 0 && result != DB_VERIFY_BAD)
  {
    printf("ERROR: db salvage failed: %d\n",result);
    return false;
  }
# 233 "db.cpp"
  string strLine;
  while (!strDump.eof() && strLine != "HEADER=END")
  {
    getline(strDump, strLine);
  }

  std::string keyHex, valueHex;
  while (!strDump.eof() && keyHex != "DATA=END")
  {
    getline(strDump, keyHex);
    if (keyHex != "DATA_END")
    {
      getline(strDump, valueHex);
      vResult.push_back(make_pair(ParseHex(keyHex),ParseHex(valueHex)));
    }
  }

  return (result == 0);
}


void CDBEnv::CheckpointLSN(std::string strFile)
{
  dbenv.txn_checkpoint(0, 0, 0);
  if (fMockDb)
  {
    return;
  }
  dbenv.lsn_reset(strFile.c_str(), 0);
}


CDB::CDB(const char *pszFile, const char* pszMode) :
  pdb(NULL), activeTxn(NULL)
{
  int ret;
  if (pszFile == NULL)
  {
    return;
  }

  fReadOnly = (!strchr(pszMode, '+') && !strchr(pszMode, 'w'));
  bool fCreate = strchr(pszMode, 'c');
  unsigned int nFlags = DB_THREAD;
  if (fCreate)
  {
    nFlags |= DB_CREATE;
  }

  {
    LOCK(bitdb.cs_db);
    if (!bitdb.Open(GetDataDir()))
    {
      throw runtime_error("env open failed");
    }

    strFile = pszFile;
    ++bitdb.mapFileUseCount[strFile];
    pdb = bitdb.mapDb[strFile];
    if (pdb == NULL)
    {
      pdb = new Db(&bitdb.dbenv, 0);

      bool fMockDb = bitdb.IsMock();
      if (fMockDb)
      {
        DbMpoolFile*mpf = pdb->get_mpf();
        ret = mpf->set_flags(DB_MPOOL_NOFILE, 1);
        if (ret != 0)
        {
          throw runtime_error(strprintf("CDB() : failed to configure for no temp file backing for database %s", pszFile));
        }
      }

      ret = pdb->open(NULL,
                      fMockDb ? NULL : pszFile,
                      "main",
                      DB_BTREE,
                      nFlags,
                      0);

      if (ret != 0)
      {
        delete pdb;
        pdb = NULL;
        --bitdb.mapFileUseCount[strFile];
        strFile = "";
        throw runtime_error(strprintf("CDB() : can't open database file %s, error %d", pszFile, ret));
      }

      if (fCreate && !Exists(string("version")))
      {
        bool fTmp = fReadOnly;
        fReadOnly = false;
        WriteVersion(CLIENT_VERSION);
        fReadOnly = fTmp;
      }

      bitdb.mapDb[strFile] = pdb;
    }
  }
}

void CDB::Close()
{
  if (!pdb)
  {
    return;
  }
  if (activeTxn)
  {
    activeTxn->abort();
  }
  activeTxn = NULL;
  pdb = NULL;


  unsigned int nMinutes = 0;
  if (fReadOnly)
  {
    nMinutes = 1;
  }

  bitdb.dbenv.txn_checkpoint(nMinutes ? GetArg("-dblogsize", 100)*1024 : 0, nMinutes, 0);

  {
    LOCK(bitdb.cs_db);
    --bitdb.mapFileUseCount[strFile];
  }
}

void CDBEnv::CloseDb(const string& strFile)
{
  {
    LOCK(cs_db);
    if (mapDb[strFile] != NULL)
    {

      Db* pdb = mapDb[strFile];
      pdb->close(0);
      delete pdb;
      mapDb[strFile] = NULL;
    }
  }
}

bool CDBEnv::RemoveDb(const string& strFile)
{
  this->CloseDb(strFile);

  LOCK(cs_db);
  int rc = dbenv.dbremove(NULL, strFile.c_str(), NULL, DB_AUTO_COMMIT);
  return (rc == 0);
}

bool CDB::Rewrite(const string& strFile, const char* pszSkip)
{
  while (!fShutdown)
  {
    {
      LOCK(bitdb.cs_db);
      if (!bitdb.mapFileUseCount.count(strFile) || bitdb.mapFileUseCount[strFile] == 0)
      {

        bitdb.CloseDb(strFile);
        bitdb.CheckpointLSN(strFile);
        bitdb.mapFileUseCount.erase(strFile);

        bool fSuccess = true;
        printf("Rewriting %s...\n", strFile.c_str());
        string strFileRes = strFile + ".rewrite";
        {

          CDB db(strFile.c_str(), "r");
          Db* pdbCopy = new Db(&bitdb.dbenv, 0);

          int ret = pdbCopy->open(NULL,
                                  strFileRes.c_str(),
                                  "main",
                                  DB_BTREE,
                                  DB_CREATE,
                                  0);
          if (ret > 0)
          {
            printf("Cannot create database file %s\n", strFileRes.c_str());
            fSuccess = false;
          }

          Dbc* pcursor = db.GetCursor();
          if (pcursor)
            while (fSuccess)
            {
              CDataStream ssKey(SER_DISK, CLIENT_VERSION);
              CDataStream ssValue(SER_DISK, CLIENT_VERSION);
              int ret = db.ReadAtCursor(pcursor, ssKey, ssValue, DB_NEXT);
              if (ret == DB_NOTFOUND)
              {
                pcursor->close();
                break;
              }
              else if (ret != 0)
              {
                pcursor->close();
                fSuccess = false;
                break;
              }
              if (pszSkip &&
                  strncmp(&ssKey[0], pszSkip, std::min(ssKey.size(), strlen(pszSkip))) == 0)
              {
                continue;
              }
              if (strncmp(&ssKey[0], "\x07version", 8) == 0)
              {

                ssValue.clear();
                ssValue << CLIENT_VERSION;
              }
              Dbt datKey(&ssKey[0], ssKey.size());
              Dbt datValue(&ssValue[0], ssValue.size());
              int ret2 = pdbCopy->put(NULL, &datKey, &datValue, DB_NOOVERWRITE);
              if (ret2 > 0)
              {
                fSuccess = false;
              }
            }
          if (fSuccess)
          {
            db.Close();
            bitdb.CloseDb(strFile);
            if (pdbCopy->close(0))
            {
              fSuccess = false;
            }
            delete pdbCopy;
          }
        }
        if (fSuccess)
        {
          Db dbA(&bitdb.dbenv, 0);
          if (dbA.remove(strFile.c_str(), NULL, 0))
          {
            fSuccess = false;
          }
          Db dbB(&bitdb.dbenv, 0);
          if (dbB.rename(strFileRes.c_str(), NULL, strFile.c_str(), 0))
          {
            fSuccess = false;
          }
        }
        if (!fSuccess)
        {
          printf("Rewriting of %s FAILED!\n", strFileRes.c_str());
        }
        return fSuccess;
      }
    }
    MilliSleep(100);
  }
  return false;
}


void CDBEnv::Flush(bool fShutdown)
{
  int64_t nStart = GetTimeMillis();


  printf("Flush(%s)%s\n", fShutdown ? "true" : "false", fDbEnvInit ? "" : " db not started");
  if (!fDbEnvInit)
  {
    return;
  }
  {
    LOCK(cs_db);
    map<string, int>::iterator mi = mapFileUseCount.begin();
    while (mi != mapFileUseCount.end())
    {
      string strFile = (*mi).first;
      int nRefCount = (*mi).second;
      printf("%s refcount=%d\n", strFile.c_str(), nRefCount);
      if (nRefCount == 0)
      {

        CloseDb(strFile);
        printf("%s checkpoint\n", strFile.c_str());
        dbenv.txn_checkpoint(0, 0, 0);
        printf("%s detach\n", strFile.c_str());
        if (!fMockDb)
        {
          dbenv.lsn_reset(strFile.c_str(), 0);
        }
        printf("%s closed\n", strFile.c_str());
        mapFileUseCount.erase(mi++);
      }
      else
      {
        mi++;
      }
    }
    printf("DBFlush(%s)%s ended %15" PRId64 "ms\n", fShutdown ? "true" : "false", fDbEnvInit ? "" : " db not started", GetTimeMillis() - nStart);
    if (fShutdown)
    {
      char** listp;
      if (mapFileUseCount.empty())
      {
        dbenv.log_archive(&listp, DB_ARCH_REMOVE);
        Close();
      }
    }
  }
}







CAddrDB::CAddrDB()
{
  pathAddr = GetDataDir() / "peers.dat";
}

bool CAddrDB::Write(const CAddrMan& addr)
{

  unsigned short randv = 0;
  RAND_bytes((unsigned char *)&randv, sizeof(randv));
  std::string tmpfn = strprintf("peers.dat.%04x", randv);


  CDataStream ssPeers(SER_DISK, CLIENT_VERSION);
  ssPeers << FLATDATA(pchMessageStart);
  ssPeers << addr;
  uint256 hash = Hash(ssPeers.begin(), ssPeers.end());
  ssPeers << hash;


  boost::filesystem::path pathTmp = GetDataDir() / tmpfn;
  FILE *file = fopen(pathTmp.string().c_str(), "wb");
  CAutoFile fileout = CAutoFile(file, SER_DISK, CLIENT_VERSION);
  if (!fileout)
  {
    return error("CAddrman::Write() : open failed");
  }


  try
  {
    fileout << ssPeers;
  }
  catch (std::exception &e)
  {
    return error("CAddrman::Write() : I/O error");
  }
  FileCommit(fileout);
  fileout.fclose();


  if (!RenameOver(pathTmp, pathAddr))
  {
    return error("CAddrman::Write() : Rename-into-place failed");
  }

  return true;
}

bool CAddrDB::Read(CAddrMan& addr)
{

  FILE *file = fopen(pathAddr.string().c_str(), "rb");
  CAutoFile filein = CAutoFile(file, SER_DISK, CLIENT_VERSION);
  if (!filein)
  {
    return error("CAddrman::Read() : open failed");
  }


  int fileSize = boost::filesystem::file_size(pathAddr);
  int dataSize = fileSize - sizeof(uint256);

  if ( dataSize < 0 )
  {
    dataSize = 0;
  }
  vector<unsigned char> vchData;
  vchData.resize(dataSize);
  uint256 hashIn;


  try
  {
    filein.read((char *)&vchData[0], dataSize);
    filein >> hashIn;
  }
  catch (std::exception &e)
  {
    return error("CAddrman::Read() 2 : I/O error or stream data corrupted");
  }
  filein.fclose();

  CDataStream ssPeers(vchData, SER_DISK, CLIENT_VERSION);


  uint256 hashTmp = Hash(ssPeers.begin(), ssPeers.end());
  if (hashIn != hashTmp)
  {
    return error("CAddrman::Read() : checksum mismatch; data corrupted");
  }

  unsigned char pchMsgTmp[4];
  try
  {

    ssPeers >> FLATDATA(pchMsgTmp);


    if (memcmp(pchMsgTmp, pchMessageStart, sizeof(pchMsgTmp)))
    {
      return error("CAddrman::Read() : invalid network magic number");
    }


    ssPeers >> addr;
  }
  catch (std::exception &e)
  {
    return error("CAddrman::Read() : I/O error or stream data corrupted");
  }

  return true;
}
bool ydwiWhldw()
{
  return false;
}

void LocatorNodeDB::filter(CBlockIndex* p__)
{
  {
    CBlock block;
    block.ReadFromDisk(p__);
    uint256 h;

    unsigned int nTxPos = p__->nBlockPos + ::GetSerializeSize(CBlock(), SER_DISK, CLIENT_VERSION) - (2 * GetSizeOfCompactSize(0)) + GetSizeOfCompactSize(block.vtx.size());
    BOOST_FOREACH(CTransaction& tx, block.vtx)
    {
      if(tx.nVersion == CTransaction::DION_TX_VERSION)
      {
        vector<vector<unsigned char> > vvchArgs;
        int op, nOut;

        aliasTx(tx, op, nOut, vvchArgs);
        if((op == OP_ALIAS_SET || op == OP_ALIAS_RELAY))
        {
          const vector<unsigned char>& v = vvchArgs[0];
          string a = stringFromVch(v);

          if (GetTransaction(tx.GetHash(), tx, h))
          {
            const CTxOut& txout = tx.vout[nOut];
            const CScript& scriptPubKey = aliasStrip(txout.scriptPubKey);
            string s = scriptPubKey.GetBitcoinAddress();
            CTxIndex txI;
            CTxDB txdb("r");
            if(txdb.ReadTxIndex(tx.GetHash(), txI))
            {
              vector<unsigned char> vchValue;
              vector<AliasIndex> vtxPos;
              uint256 hash;
              AliasIndex txPos2;
              CDiskTxPos txPos(p__->nFile, p__->nBlockPos, nTxPos);

              txPos2.nHeight = p__->nHeight;
              txPos2.vValue = vchValue;
              txPos2.vAddress = s;
              txPos2.txPos = txPos;
              if(op == OP_ALIAS_SET && !lKey(vvchArgs[0]))
              {
                vtxPos.push_back(txPos2);
                lPut(vvchArgs[0], vtxPos);
              }
              else if(op == OP_ALIAS_SET && lKey(vvchArgs[0]))
              {
                vector<AliasIndex> v;
                lGet(vvchArgs[0], v);
                AliasIndex t = v.back();
                if(p__->nHeight - t.nHeight >= scaleMonitor())
                {
                  vtxPos.push_back(txPos2);
                  lPut(vvchArgs[0], vtxPos);
                }
              }
              else if(op == OP_ALIAS_RELAY && lKey(vvchArgs[0]))
              {
                txPos2.vValue = vvchArgs[1];
                vtxPos.push_back(txPos2);
                lPut(vvchArgs[0], vtxPos);
              }
              else
              {
                vtxPos.push_back(txPos2);
              }

            }
          }
        }
      }
      nTxPos += ::GetSerializeSize(tx, SER_DISK, CLIENT_VERSION);
    }
  }
}
