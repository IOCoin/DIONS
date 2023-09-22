#pragma once

#include <string>
#include <vector>

#include <stdlib.h>

#include "ccoin/process.h"
#include "crypto/key.h"
#include "keystore.h"
#include "script.h"
#include "ui_interface.h"
#include "core/util.h"
#include "walletdb.h"
#include "api_transaction.h"
#include "wallet_transaction.h"

#include "ray_shade.h"
#include "rpc/wallet_face.h"

class CAccountingEntry;
class __wx__Tx;
class CReserveKey;
class COutput;
class CCoinControl;


enum WalletFeature
{
    FEATURE_BASE = 10500,

    FEATURE_WALLETCRYPT = 40000,
    FEATURE_COMPRPUBKEY = 60000,
    FEATURE_SHADE = 80000,

    FEATURE_LATEST = FEATURE_SHADE
};


bool __intersect(CKeyID& i, CPubKey& j);


class CKeyPool
{
public:
    int64_t nTime;
    CPubKey vchPubKey;

    CKeyPool()
    {
        nTime = GetTime();
    }

    CKeyPool(const CPubKey& vchPubKeyIn)
    {
        nTime = GetTime();
        vchPubKey = vchPubKeyIn;
    }

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(nTime);
        READWRITE(vchPubKey);
    )
};




class __wx__ : public CCryptoKeyStore, public WalletFace
{
private:
    bool SelectCoinsForStaking(int64_t nTargetValue, unsigned int nSpendTime, std::set<std::pair<const __wx__Tx*,unsigned int> >& setCoinsRet, int64_t& nValueRet) const;

    __wx__DB *pwalletdbEncryption;


    int nWalletVersion;


    int nWalletMaxVersion;

public:
    bool SelectCoins(int64_t nTargetValue, unsigned int nSpendTime, std::set<std::pair<const __wx__Tx*,unsigned int> >& setCoinsRet, int64_t& nValueRet, const CCoinControl *coinControl=NULL) const;

    virtual json_spirit::Value getBalance(const json_spirit::Array&, bool) override;

    mutable CCriticalSection cs_wallet;

    bool fFileBacked;
    std::string strWalletFile;
    std::set<int64_t> setKeyPool;
    std::map<CKeyID, CKeyMetadata> kd;
    std::map<vchType, Relay> lCache;

    typedef std::map<unsigned int, CMasterKey> MasterKeyMap;
    MasterKeyMap mapMasterKeys;
    unsigned int nMasterKeyMaxID;

    __wx__()
    {
        SetNull();
    }
    __wx__(std::string strWalletFileIn)
    {
        SetNull();

        strWalletFile = strWalletFileIn;
        fFileBacked = true;
    }
    void SetNull()
    {
        nWalletVersion = FEATURE_BASE;
        nWalletMaxVersion = FEATURE_BASE;
        fFileBacked = false;
        nMasterKeyMaxID = 0;
        pwalletdbEncryption = NULL;
        nOrderPosNext = 0;
        nTimeFirstKey = 0;
    }

    std::map<uint256, __wx__Tx> mapWallet;
    int64_t nOrderPosNext;
    std::map<uint256, int> mapRequestCount;

    std::map<CTxDestination, std::string> mapAddressBook;

    CPubKey vchDefaultKey;
    int64_t nTimeFirstKey;

    bool CanSupportFeature(enum WalletFeature wf) { AssertLockHeld(cs_wallet); return nWalletMaxVersion >= wf; }

    void AvailableCoinsForStaking(std::vector<COutput>& vCoins, unsigned int nSpendTime) const;
    void AvailableCoins(std::vector<COutput>& vCoins, bool fOnlyConfirmed=true, const CCoinControl *coinControl=NULL) const;
    bool SelectCoinsMinConf(int64_t nTargetValue, unsigned int nSpendTime, int nConfMine, int nConfTheirs, std::vector<COutput> vCoins, std::set<std::pair<const __wx__Tx*,unsigned int> >& setCoinsRet, int64_t& nValueRet) const;



    CPubKey GenerateNewKey();

    bool ak(const CKey& key);

    bool LoadKey(const CKey& key) { return CCryptoKeyStore::ak(key); }

    bool envCP0(const CPubKey &pubkey, string& rsaPrivKey);
    bool envCP1(const CPubKey &pubkey, string& rsaPubKey);
    bool aes_(const CPubKey &pubkey, string& f, string& aesPlainBase64);
    bool aes(const CPubKey &pubkey, string& f, string& aes256KeyBase64);
    bool SetRSAMetadata(const CPubKey &pubkey);
    bool GetRandomKeyMetadata(const CPubKey& pubkey, vchType &r, string& r_);
    bool relay_(const vchType &k, Relay& r);
    bool relay(const vchType &k, Relay& r);
    bool vtx_(const CPubKey &pubkey, string& v);
    bool vtx(const CPubKey &pubkey, string& v);
    bool SetRandomKeyMetadata(const CPubKey& pubkey, const vchType &r);
    bool LoadKeyMetadata(const CPubKey &pubkey, const CKeyMetadata &metadata);

    bool LoadMinVersion(int nVersion) { AssertLockHeld(cs_wallet); nWalletVersion = nVersion; nWalletMaxVersion = std::max(nWalletMaxVersion, nVersion); return true; }


    bool sync(const CPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret);

    bool LoadCryptedKey(const CPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret);
    bool AddCScript(const CScript& redeemScript);
    bool LoadCScript(const CScript& redeemScript);


    bool Unlock(const SecureString& strWalletPassphrase);
    bool LoadRelay(const vchType& k, const Relay& r);
    bool ChangeWalletPassphrase(const SecureString& strOldWalletPassphrase, const SecureString& strNewWalletPassphrase);
    bool EncryptWallet(const SecureString& strWalletPassphrase);

    void kt(std::map<CKeyID, int64_t> &mapKeyBirth) const;





    int64_t IncOrderPosNext(__wx__DB *pwalletdb = NULL);

    typedef std::pair<__wx__Tx*, CAccountingEntry*> TxPair;
    typedef std::multimap<int64_t, TxPair > TxItems;





    TxItems OrderedTxItems(std::list<CAccountingEntry>& acentries, std::string strAccount = "");

    void MarkDirty();
    bool AddToWallet(const __wx__Tx& wtxIn);
    bool AddToWalletIfInvolvingMe(const CTransaction& tx, const CBlock* pblock, bool fUpdate = false, bool fFindBlock = false);
    bool EraseFromWallet(uint256 hash);
    void WalletUpdateSpent(const CTransaction& prevout, bool fBlock = false);
    int ScanForWalletTransactions(CBlockIndex* pindexStart, bool fUpdate = false);
    void ReacceptWalletTransactions();
    void ResendWalletTransactions(bool fForce = false);
    int64_t GetBalance() const;
    int64_t GetUnconfirmedBalance() const;
    int64_t ImmatureBalance() const;
    bool __transient();
    int64_t GetStake() const;
    int64_t GetNewMint() const;
    const boost::filesystem::path& dataDir() const
    {
      return GetDataDir();
    }
    bool CreateTransaction(const std::vector<std::pair<CScript, int64_t> >& vecSend, __wx__Tx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string strTxInfo, const CCoinControl *coinControl=NULL);
    bool CreateTransaction__(const std::vector<std::pair<CScript, int64_t> >& vecSend, __wx__Tx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string strTxInfo, const CCoinControl *coinControl=NULL);
    bool CreateTransaction(CScript scriptPubKey, int64_t nValue, __wx__Tx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string strTxInfo, const CCoinControl *coinControl=NULL);
    bool CreateTransaction__(CScript scriptPubKey, int64_t nValue, __wx__Tx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string strTxInfo, const CCoinControl *coinControl=NULL);
    bool CommitTransaction(__wx__Tx& wtxNew, CReserveKey& reservekey);
    bool CommitTransaction__(__wx__Tx& wtxNew, CReserveKey& reservekey);

    bool GetStakeWeight(uint64_t& nWeight);
    bool CreateCoinStake(const CKeyStore& keystore, unsigned int nBits, int64_t nSearchInterval, int64_t nFees, CTransaction& txNew, CKey& key, int nHeight);

    std::string SendMoney(CScript scriptPubKey, int64_t nValue, __wx__Tx& wtxNew, bool fAskFee=false,std::string strTx="");
    std::string SendMoney__(CScript scriptPubKey, int64_t nValue, __wx__Tx& wtxNew, bool fAskFee=false,std::string strTx="");
    std::string SendMoneyToDestination(const CTxDestination &address, int64_t nValue, __wx__Tx& wtxNew, bool fAskFee=false,std::string strTx="");

    bool NewKeyPool();
    bool TopUpKeyPool(unsigned int nSize = 0);
    std::string __associate_fn__(CScript,int64_t,__wx__Tx&,__im__&);
    bool __x_form__(CScript,int64_t,__im__&,__wx__Tx&,int64_t&,CReserveKey&);
    int64_t AddReserveKey(const CKeyPool& keypool);
    void ReserveKeyFromKeyPool(int64_t& nIndex, CKeyPool& keypool);
    void KeepKey(int64_t nIndex);
    void ReturnKey(int64_t nIndex);
    bool GetKeyFromPool(CPubKey &key, bool fAllowReuse=true);
    bool GetKeyFromPool(CPubKey&, CPubKey&, bool fAllowReuse=true);
    vector<unsigned char> GetKeyFromKeyPool();
    int64_t GetOldestKeyPoolTime();
    void GetAllReserveKeys(std::set<CKeyID>& setAddress) const;

    std::set< std::set<CTxDestination> > GetAddressGroupings();
    std::map<CTxDestination, int64_t> GetAddressBalances();

    bool __xfa(const vector<CTxOut>& vout);
    bool IsMine(const CTxIn& txin) const;
    int64_t GetDebit(const CTxIn& txin) const;
    bool IsMine(const CTxOut& txout) const
    {
        return ::IsMine(*this, txout.scriptPubKey);
    }
    int64_t GetCredit(const CTxOut& txout) const
    {
        if (!MoneyRange(txout.nValue))
            throw std::runtime_error("__wx__::GetCredit() : value out of range");

        if(IsMine(txout))
          return txout.nValue;

        return 0;
    }
    bool IsChange(const CTxOut& txout) const;
    int64_t GetChange(const CTxOut& txout) const
    {
        if (!MoneyRange(txout.nValue))
            throw std::runtime_error("__wx__::GetChange() : value out of range");
        return (IsChange(txout) ? txout.nValue : 0);
    }
    bool IsMine(const CTransaction& tx) 
    {
        bool s = __xfa(tx.vout);
        BOOST_FOREACH(const CTxOut& txout, tx.vout)
        {
            if (IsMine(txout) && tx.nVersion == CTransaction::DION_TX_VERSION && txout.nValue >= nMinimumInputValue)
                return true;
            else if (IsMine(txout) && txout.nValue > nMinimumInputValue)
                return true;

        }

        return s;
    }
    bool IsFromMe(const CTransaction& tx) const
    {
        return (GetDebit(tx) > 0);
    }
    int64_t GetDebit(const CTransaction& tx) const
    {
        int64_t nDebit = 0;
        BOOST_FOREACH(const CTxIn& txin, tx.vin)
        {
            nDebit += GetDebit(txin);
            if (!MoneyRange(nDebit))
                throw std::runtime_error("__wx__::GetDebit() : value out of range");
        }
        return nDebit;
    }
    int64_t GetCredit(const CTransaction& tx) const
    {
        int64_t nCredit = 0;
        BOOST_FOREACH(const CTxOut& txout, tx.vout)
        {
            nCredit += GetCredit(txout);
            if (!MoneyRange(nCredit))
                throw std::runtime_error("__wx__::GetCredit() : value out of range");
        }
        return nCredit;
    }
    int64_t GetChange(const CTransaction& tx) const
    {
        int64_t nChange = 0;
        BOOST_FOREACH(const CTxOut& txout, tx.vout)
        {
            nChange += GetChange(txout);
            if (!MoneyRange(nChange))
                throw std::runtime_error("__wx__::GetChange() : value out of range");
        }
        return nChange;
    }
    void SetBestChain(const CBlockLocator& loc);

    DBErrors LoadWallet(bool& fFirstRunRet);

    DBErrors ZapWalletTx();

    bool SetAddressBookName(const CTxDestination& address, const std::string& aliasStr);

    bool DelAddressBookName(const CTxDestination& address);

    void UpdatedTransaction(const uint256 &hashTx);

    void PrintWallet(const CBlock& block);

    void Inventory(const uint256 &hash)
    {
        {
            LOCK(cs_wallet);
            std::map<uint256, int>::iterator mi = mapRequestCount.find(hash);
            if (mi != mapRequestCount.end())
                (*mi).second++;
        }
    }

    unsigned int GetKeyPoolSize()
    {
        AssertLockHeld(cs_wallet);
        return setKeyPool.size();
    }

    bool GetTransaction(const uint256 &hashTx, __wx__Tx& wtx);

    bool SetDefaultKey(const CPubKey &vchPubKey);


    bool SetMinVersion(enum WalletFeature, __wx__DB* pwalletdbIn = NULL, bool fExplicit = false);


    bool SetMaxVersion(int nVersion);


    int GetVersion() { LOCK(cs_wallet); return nWalletVersion; }
    bool __intersect(CKeyID& i, CPubKey& j);


    void FixSpentCoins(int& nMismatchSpent, int64_t& nBalanceInQuestion, bool fCheckOnly = false);
    void DisableTransaction(const CTransaction &tx);




    boost::signals2::signal<void (__wx__ *wallet, const CTxDestination &address, const std::string &label, bool isMine, ChangeType status)> NotifyAddressBookChanged;




    boost::signals2::signal<void (__wx__ *wallet, const uint256 &hashTx, ChangeType status)> NotifyTransactionChanged;
};


class CReserveKey
{
protected:
    __wx__* pwallet;
    int64_t nIndex;
    CPubKey vchPubKey;
public:
    CReserveKey(__wx__* pwalletIn)
    {
        nIndex = -1;
        pwallet = pwalletIn;
    }

    ~CReserveKey()
    {
        if (!fShutdown)
            ReturnKey();
    }

    void ReturnKey();
    bool GetReservedKey(CPubKey &pubkey);
    void KeepKey();
};


typedef std::map<std::string, std::string> mapValue_t;


void ReadOrderPos(int64_t& nOrderPos, mapValue_t& mapValue);
void WriteOrderPos(const int64_t& nOrderPos, mapValue_t& mapValue);

class __wx__Key
{
public:
    CPrivKey vchPrivKey;
    int64_t nTimeCreated;
    int64_t nTimeExpires;
    std::string strComment;

    __wx__Key(int64_t nExpires=0)
    {
        nTimeCreated = (nExpires ? GetTime() : 0);
        nTimeExpires = nExpires;
    }

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vchPrivKey);
        READWRITE(nTimeCreated);
        READWRITE(nTimeExpires);
        READWRITE(strComment);
    )
};
class CAccount
{
public:
    CPubKey vchPubKey;

    CAccount()
    {
        SetNull();
    }

    void SetNull()
    {
        vchPubKey = CPubKey();
    }

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vchPubKey);
    )
};






class CAccountingEntry
{
public:
    std::string strAccount;
    int64_t nCreditDebit;
    int64_t nTime;
    std::string strOtherAccount;
    std::string strComment;
    mapValue_t mapValue;
    int64_t nOrderPos;
    uint64_t nEntryNo;

    CAccountingEntry()
    {
        SetNull();
    }

    void SetNull()
    {
        nCreditDebit = 0;
        nTime = 0;
        strAccount.clear();
        strOtherAccount.clear();
        strComment.clear();
        nOrderPos = -1;
    }

    IMPLEMENT_SERIALIZE
    (
        CAccountingEntry& me = *const_cast<CAccountingEntry*>(this);
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);

        READWRITE(nCreditDebit);
        READWRITE(nTime);
        READWRITE(strOtherAccount);

        if (!fRead)
        {
            WriteOrderPos(nOrderPos, me.mapValue);

            if (!(mapValue.empty() && _ssExtra.empty()))
            {
                CDataStream ss(nType, nVersion);
                ss.insert(ss.begin(), '\0');
                ss << mapValue;
                ss.insert(ss.end(), _ssExtra.begin(), _ssExtra.end());
                me.strComment.append(ss.str());
            }
        }

        READWRITE(strComment);

        size_t nSepPos = strComment.find("\0", 0, 1);
        if (fRead)
        {
            me.mapValue.clear();
            if (std::string::npos != nSepPos)
            {
                CDataStream ss(std::vector<char>(strComment.begin() + nSepPos + 1, strComment.end()), nType, nVersion);
                ss >> me.mapValue;
                me._ssExtra = std::vector<char>(ss.begin(), ss.end());
            }
            ReadOrderPos(me.nOrderPos, me.mapValue);
        }
        if (std::string::npos != nSepPos)
            me.strComment.erase(nSepPos);

        me.mapValue.erase("n");
    )

private:
    std::vector<char> _ssExtra;
};

bool GetWalletFile(__wx__* pwallet, std::string &strWalletFileOut);

