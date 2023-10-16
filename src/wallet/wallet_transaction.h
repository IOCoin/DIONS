#pragma once

#include "wallet.h"
#include "TypeMapValue.h"
class CCoinControl;
class COutput;
extern bool fWalletUnlockStakingOnly;
extern bool fConfChange;
extern void ReadOrderPos(int64_t& nOrderPos, mapValue_t& mapValue);
extern void WriteOrderPos(const int64_t& nOrderPos, mapValue_t& mapValue);
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

    int64_t GetDebit() const;

    int64_t GetCredit(bool fUseCache=true) const;

    int64_t GetAvailableCredit(bool fUseCache=true) const;


    int64_t GetChange() const;

    void GetAmounts(std::list<std::pair<CTxDestination, int64_t> >& listReceived,
                    std::list<std::pair<CTxDestination, int64_t> >& listSent, int64_t& nFee, std::string& strSentAccount) const;

    void GetAccountAmounts(const std::string& strAccount, int64_t& nReceived,
                           int64_t& nSent, int64_t& nFee) const;

    bool IsFromMe() const
    {
        return (GetDebit() > 0);
    }

    bool IsTrusted() const;

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

