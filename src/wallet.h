#ifndef BITCOIN_WALLET_H
#define BITCOIN_WALLET_H 

#include <string>
#include <vector>

#include <stdlib.h>

#include "main.h"
#include "key.h"
#include "keystore.h"
#include "script.h"
#include "ui_interface.h"
#include "util.h"
#include "walletdb.h"
#include "api_transaction.h"

#include "ray_shade.h"

extern bool fWalletUnlockStakingOnly;
extern bool fConfChange;
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




class __wx__ : public CCryptoKeyStore
{
private:
    bool SelectCoinsForStaking(int64_t nTargetValue, unsigned int nSpendTime, std::set<std::pair<const __wx__Tx*,unsigned int> >& setCoinsRet, int64_t& nValueRet) const;

    __wx__DB *pwalletdbEncryption;


    int nWalletVersion;


    int nWalletMaxVersion;

public:
    bool SelectCoins(int64_t nTargetValue, unsigned int nSpendTime, std::set<std::pair<const __wx__Tx*,unsigned int> >& setCoinsRet, int64_t& nValueRet, const CCoinControl *coinControl=NULL) const;





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

    bool __xfa(const vector<CTxOut>& vout) const;
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
    bool IsMine(const CTransaction& tx) const
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


static void ReadOrderPos(int64_t& nOrderPos, mapValue_t& mapValue)
{
    if (!mapValue.count("n"))
    {
        nOrderPos = -1;
        return;
    }
    nOrderPos = atoi64(mapValue["n"].c_str());
}


static void WriteOrderPos(const int64_t& nOrderPos, mapValue_t& mapValue)
{
    if (nOrderPos == -1)
        return;
    mapValue["n"] = i64tostr(nOrderPos);
}





class __wx__Tx : public CMerkleTx
{
private:

public:
    const __wx__* pwallet;
    std::vector<CMerkleTx> vtxPrev;
    mapValue_t mapValue;
    std::vector<std::pair<std::string, std::string> > vOrderForm;
    unsigned int fTimeReceivedIsTxTime;
    unsigned int nTimeReceived;
    unsigned int nTimeSmart;
    char fFromMe;
    std::string strFromAccount;
    std::vector<char> vfSpent;
    int64_t nOrderPos;


    mutable bool fDebitCached;
    mutable bool fCreditCached;
    mutable bool fAvailableCreditCached;
    mutable bool fChangeCached;
    mutable int64_t nDebitCached;
    mutable int64_t nCreditCached;
    mutable int64_t nAvailableCreditCached;
    mutable int64_t nChangeCached;

    mutable int nAliasOut;
    mutable vchType vchAlias;
    mutable vchType vchValue;
    mutable int op__;
    mutable bool s__;

    mutable bool messageTxDecoded;
    mutable bool messageTxDecodeSuccess;
    mutable int nMessageOut;
    mutable vchType vchMessage;

    mutable bool pkTxDecoded;
    mutable bool pkTxDecodeSuccess;
    mutable int nPKOut;
    mutable vchType vchSender;
    mutable vchType vchRecipient;
    mutable vchType vchKey;
    mutable vchType vchAESKeyEncrypted;
    mutable vchType iv128Base64Vch;
    mutable vchType vchSignature;

    __wx__Tx()
    {
        Init(NULL);
    }

    __wx__Tx(const __wx__* pwalletIn)
    {
        Init(pwalletIn);
    }

    __wx__Tx(const __wx__* pwalletIn, const CMerkleTx& txIn) : CMerkleTx(txIn)
    {
        Init(pwalletIn);
    }

    __wx__Tx(const __wx__* pwalletIn, const CTransaction& txIn) : CMerkleTx(txIn)
    {
        Init(pwalletIn);
    }

    void Init(const __wx__* pwalletIn)
    {
        pwallet = pwalletIn;
        vtxPrev.clear();
        mapValue.clear();
        vOrderForm.clear();
        fTimeReceivedIsTxTime = false;
        nTimeReceived = 0;
        nTimeSmart = 0;
        fFromMe = false;
        strFromAccount.clear();
        vfSpent.clear();
        fDebitCached = false;
        fCreditCached = false;
        fAvailableCreditCached = false;
        fChangeCached = false;
        nDebitCached = 0;
        nCreditCached = 0;
        nAvailableCreditCached = 0;
        nChangeCached = 0;
        nOrderPos = -1;
    }

    IMPLEMENT_SERIALIZE
    (
        __wx__Tx* pthis = const_cast<__wx__Tx*>(this);
        if (fRead)
            pthis->Init(NULL);
        char fSpent = false;

        if (!fRead)
        {
            pthis->mapValue["fromaccount"] = pthis->strFromAccount;

            std::string str;
            BOOST_FOREACH(char f, vfSpent)
            {
                str += (f ? '1' : '0');
                if (f)
                    fSpent = true;
            }
            pthis->mapValue["spent"] = str;

            WriteOrderPos(pthis->nOrderPos, pthis->mapValue);

            if (nTimeSmart)
                pthis->mapValue["timesmart"] = strprintf("%u", nTimeSmart);
        }

        nSerSize += SerReadWrite(s, *(CMerkleTx*)this, nType, nVersion,ser_action);
        READWRITE(vtxPrev);
        READWRITE(mapValue);
        READWRITE(vOrderForm);
        READWRITE(fTimeReceivedIsTxTime);
        READWRITE(nTimeReceived);
        READWRITE(fFromMe);
        READWRITE(fSpent);

        if (fRead)
        {
            pthis->strFromAccount = pthis->mapValue["fromaccount"];

            if (mapValue.count("spent"))
                BOOST_FOREACH(char c, pthis->mapValue["spent"])
                    pthis->vfSpent.push_back(c != '0');
            else
                pthis->vfSpent.assign(vout.size(), fSpent);

            ReadOrderPos(pthis->nOrderPos, pthis->mapValue);

            pthis->nTimeSmart = mapValue.count("timesmart") ? (unsigned int)atoi64(pthis->mapValue["timesmart"]) : 0;
        }

        pthis->mapValue.erase("fromaccount");
        pthis->mapValue.erase("version");
        pthis->mapValue.erase("spent");
        pthis->mapValue.erase("n");
        pthis->mapValue.erase("timesmart");
    )



    bool UpdateSpent(const std::vector<char>& vfNewSpent)
    {
        bool fReturn = false;
        for (unsigned int i = 0; i < vfNewSpent.size(); i++)
        {
            if (i == vfSpent.size())
                break;

            if (vfNewSpent[i] && !vfSpent[i])
            {
                vfSpent[i] = true;
                fReturn = true;
                fAvailableCreditCached = false;
            }
        }
        return fReturn;
    }


    void MarkDirty()
    {
        fCreditCached = false;
        fAvailableCreditCached = false;
        fDebitCached = false;
        fChangeCached = false;
    }

    void BindWallet(__wx__ *pwalletIn)
    {
        pwallet = pwalletIn;
        MarkDirty();
    }

    void MarkSpent(unsigned int nOut)
    {

        if (nOut >= vout.size())
            throw std::runtime_error("__wx__Tx::MarkSpent() : nOut out of range");
        vfSpent.resize(vout.size());
        if (!vfSpent[nOut])
        {
            vfSpent[nOut] = true;
            fAvailableCreditCached = false;
        }
    }

    void MarkUnspent(unsigned int nOut)
    {
        if (nOut >= vout.size())
            throw std::runtime_error("__wx__Tx::MarkUnspent() : nOut out of range");
        vfSpent.resize(vout.size());
        if (vfSpent[nOut])
        {
            vfSpent[nOut] = false;
            fAvailableCreditCached = false;
        }
    }

    bool IsSpent(unsigned int nOut) const
    {
        if (nOut >= vout.size())
            throw std::runtime_error("__wx__Tx::IsSpent() : nOut out of range");
        if (nOut >= vfSpent.size())
            return false;
        return (!!vfSpent[nOut]);
    }

    int64_t GetDebit() const
    {
        if (vin.empty())
            return 0;
        if (fDebitCached)
            return nDebitCached;
        nDebitCached = pwallet->GetDebit(*this);
        fDebitCached = true;
        return nDebitCached;
    }

    int64_t GetCredit(bool fUseCache=true) const
    {

        if ((IsCoinBase() || IsCoinStake()) && GetBlocksToMaturity() > 0)
            return 0;


        if (fUseCache && fCreditCached)
            return nCreditCached;
        nCreditCached = pwallet->GetCredit(*this);
        fCreditCached = true;
        return nCreditCached;
    }

    int64_t GetAvailableCredit(bool fUseCache=true) const
    {

        if ((IsCoinBase() || IsCoinStake()) && GetBlocksToMaturity() > 0)
            return 0;

        if (fUseCache && fAvailableCreditCached)
            return nAvailableCreditCached;

        int64_t nCredit = 0;
        for (unsigned int i = 0; i < vout.size(); i++)
        {
            if (!IsSpent(i))
            {
                const CTxOut &txout = vout[i];
                nCredit += pwallet->GetCredit(txout);
                if (!MoneyRange(nCredit))
                    throw std::runtime_error("__wx__Tx::GetAvailableCredit() : value out of range");
            }
        }

        nAvailableCreditCached = nCredit;
        fAvailableCreditCached = true;
        return nCredit;
    }


    int64_t GetChange() const
    {
        if (fChangeCached)
            return nChangeCached;
        nChangeCached = pwallet->GetChange(*this);
        fChangeCached = true;
        return nChangeCached;
    }

    void GetAmounts(std::list<std::pair<CTxDestination, int64_t> >& listReceived,
                    std::list<std::pair<CTxDestination, int64_t> >& listSent, int64_t& nFee, std::string& strSentAccount) const;

    void GetAccountAmounts(const std::string& strAccount, int64_t& nReceived,
                           int64_t& nSent, int64_t& nFee) const;

    bool IsFromMe() const
    {
        return (GetDebit() > 0);
    }

    bool IsTrusted() const
    {

        if (!IsFinalTx(*this))
            return false;
        int nDepth = GetDepthInMainChain();
        if (nDepth >= 1)
        {
            return true;
        }
        if (nDepth < 0)
        {
            return false;
        }
        if (fConfChange || !IsFromMe())
            return false;



        std::map<uint256, const CMerkleTx*> mapPrev;
        std::vector<const CMerkleTx*> vWorkQueue;
        vWorkQueue.reserve(vtxPrev.size()+1);
        vWorkQueue.push_back(this);
        for (unsigned int i = 0; i < vWorkQueue.size(); i++)
        {
            const CMerkleTx* ptx = vWorkQueue[i];

            if (!IsFinalTx(*ptx))
            {
                return false;
            }
            int nPDepth = ptx->GetDepthInMainChain();
            if (nPDepth >= 1)
            {
                continue;
            }
            if (nPDepth < 0)
            {
                return false;
            }
            if (!pwallet->IsFromMe(*ptx))
            {
                return false;
             }

            if (mapPrev.empty())
            {
                BOOST_FOREACH(const CMerkleTx& tx, vtxPrev)
                    mapPrev[tx.GetHash()] = &tx;
            }

            BOOST_FOREACH(const CTxIn& txin, ptx->vin)
            {
                if (!mapPrev.count(txin.prevout.hash))
                {
                    return false;
                }
                vWorkQueue.push_back(mapPrev[txin.prevout.hash]);
            }
        }

        return true;
    }

    bool GetEncryptedMessageUpdate(int& nOut, vchType& nm, vchType& r, vchType& val, vchType& iv, vchType& s) const;
    bool GetMessageUpdate (int& nOut, vchType& nm, vchType& r, vchType& val, vchType& s) const;
    bool GetPublicKeyUpdate (int& nOut, vchType& nm, vchType& r, vchType& val, vchType& aes, vchType& s) const;
    bool vtx (int& nOut, vchType& nm, vchType& r, vchType& val, vchType& aes, vchType& s) const;
    bool proj(int& nOut, vchType& nm, vchType& r, vchType& val, vchType& iv, vchType& s) const;
    bool aliasSet(int& r, int& p, vector< vector<unsigned char> >& vv) const;
    bool aliasSet(int& r, int& p, vchType& v1, vchType& val) const;
    bool aliasStream(int& r, int& p, vchType& v1, vchType& val, vchType& vchS, vchType& inV3) const;

    bool WriteToDisk();

    int64_t GetTxTime() const;
    int GetRequestCount() const;

    void AddSupportingTransactions(CTxDB& txdb);

    bool AcceptWalletTransaction(CTxDB& txdb);
    bool AcceptWalletTransaction();

    void RelayWalletTransaction(CTxDB& txdb);
    void RelayWalletTransaction();
};




class COutput
{
public:
    const __wx__Tx *tx;
    int i;
    int nDepth;

    COutput(const __wx__Tx *txIn, int iIn, int nDepthIn)
    {
        tx = txIn; i = iIn; nDepth = nDepthIn;
    }

    std::string ToString() const
    {
        return strprintf("COutput(%s, %d, %d) [%s]", tx->GetHash().ToString().substr(0,10).c_str(), i, nDepth, FormatMoney(tx->vout[i].nValue).c_str());
    }

    void print() const
    {
        printf("%s\n", ToString().c_str());
    }
};





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
# 872 "wallet.h"
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

#endif
