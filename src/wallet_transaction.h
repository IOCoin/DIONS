#ifndef WALLET_TX_H
#define WALLET_TX_H

#include <vector>
#include "api_transaction.h"
#include "type_mapvalue.h"

extern bool fConfChange;
class __wx__Tx : public CMerkleTx
{
private:

public:
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
        Init();
    }

    __wx__Tx(const CMerkleTx& txIn) : CMerkleTx(txIn)
    {
        Init();
    }

    __wx__Tx(const CTransaction& txIn) : CMerkleTx(txIn)
    {
        Init();
    }

    void Init()
    {
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
            pthis->Init();
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

            if (nOrderPos != -1)
              pthis->mapValue["n"] = i64tostr(nOrderPos);

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

    if (!pthis->mapValue.count("n"))
    {
        pthis->nOrderPos = -1;
    }
    else
      pthis->nOrderPos = atoi64(pthis->mapValue["n"].c_str());

            pthis->nTimeSmart = mapValue.count("timesmart") ? (unsigned int)atoi64(pthis->mapValue["timesmart"]) : 0;
        }

        pthis->mapValue.erase("fromaccount");
        pthis->mapValue.erase("version");
        pthis->mapValue.erase("spent");
        pthis->mapValue.erase("n");
        pthis->mapValue.erase("timesmart");
    )



    bool UpdateSpent(const std::vector<char>& vfNewSpent);
    void MarkDirty();
    void BindWallet();
    void MarkSpent(unsigned int nOut);
    void MarkUnspent(unsigned int nOut);
    bool IsSpent(unsigned int nOut) const;
    int64_t GetDebit() const;
    int64_t GetCredit(bool fUseCache=true) const;
    int64_t GetChange() const;
    bool IsFromMe() const;
    bool GetEncryptedMessageUpdate(int& nOut, vchType& nm, vchType& r, vchType& val, vchType& iv, vchType& s) const;
    bool GetMessageUpdate (int& nOut, vchType& nm, vchType& r, vchType& val, vchType& s) const;
    bool GetPublicKeyUpdate (int& nOut, vchType& nm, vchType& r, vchType& val, vchType& aes, vchType& s) const;
    bool vtx (int& nOut, vchType& nm, vchType& r, vchType& val, vchType& aes, vchType& s) const;
    bool proj(int& nOut, vchType& nm, vchType& r, vchType& val, vchType& iv, vchType& s) const;
    bool aliasSet(int& r, int& p, vector< vector<unsigned char> >& vv) const;
    bool aliasSet(int& r, int& p, vchType& v1, vchType& val) const;
    bool aliasStream(int& r, int& p, vchType& v1, vchType& val, vchType& vchS, vchType& inV3) const;
    int64_t GetTxTime() const;
    int GetRequestCount() const;
    bool AcceptWalletTransaction(CTxDB& txdb);
    bool AcceptWalletTransaction();

    void RelayWalletTransaction(CTxDB& txdb);
    void RelayWalletTransaction();
};


#endif
