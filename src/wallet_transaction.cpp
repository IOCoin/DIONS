
#include "wallet_transaction.h"

int64_t __wx__Tx::GetTxTime() const
{
  int64_t n = nTimeSmart;
  return n ? n : nTimeReceived;
}
    bool __wx__Tx::UpdateSpent(const std::vector<char>& vfNewSpent)
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


    void __wx__Tx::MarkDirty()
    {
        fCreditCached = false;
        fAvailableCreditCached = false;
        fDebitCached = false;
        fChangeCached = false;
    }

    void __wx__Tx::BindWallet()
    {
        MarkDirty();
    }

    void __wx__Tx::MarkSpent(unsigned int nOut)
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

    void __wx__Tx::MarkUnspent(unsigned int nOut)
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

    bool __wx__Tx::IsSpent(unsigned int nOut) const
    {
        if (nOut >= vout.size())
            throw std::runtime_error("__wx__Tx::IsSpent() : nOut out of range");
        if (nOut >= vfSpent.size())
            return false;
        return (!!vfSpent[nOut]);
    }

    int64_t __wx__Tx::GetDebit() const
    {
        if (vin.empty())
            return 0;
        if (fDebitCached)
            return nDebitCached;
        fDebitCached = true;
        return nDebitCached;
    }

    int64_t __wx__Tx::GetCredit(bool fUseCache) const
    {

        if ((IsCoinBase() || IsCoinStake()) && GetBlocksToMaturity() > 0)
            return 0;


        if (fUseCache && fCreditCached)
            return nCreditCached;
        fCreditCached = true;
        return nCreditCached;
    }

    int64_t __wx__Tx::GetChange() const
    {
        if (fChangeCached)
            return nChangeCached;
        fChangeCached = true;
        return nChangeCached;
    }

    bool __wx__Tx::IsFromMe() const
    {
        return (GetDebit() > 0);
    }
