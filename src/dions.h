#ifndef NAMECOIN_H
#define NAMECOIN_H

#include "json/json_spirit.h"

static const int64_t MIN_AMOUNT = CENT;
static const int MAX_NAME_LENGTH = 255;
static const int MAX_VALUE_LENGTH = 1023;
static const int OP_NAME_INVALID = 0x00;
static const int OP_NAME_NEW = 0x01;
static const int OP_NAME_FIRSTUPDATE = 0x02;
static const int OP_NAME_UPDATE = 0x03;
static const int OP_NAME_NOP = 0x04;
static const int OP_MESSAGE = 0x05;
static const int OP_ENCRYPTED_MESSAGE = 0x06;
static const int OP_PUBLIC_KEY = 0x07;
static const int MIN_FIRSTUPDATE_DEPTH = 1;

/* Maximum value length that is allowed by the UIs.  Currently,
   if the value is set above 520 bytes, it can't ever be updated again
   due to limitations in the scripting system.  Enforce this
   in the UIs.  */
static const int UI_MAX_VALUE_LENGTH = 520;

//typedef std::vector<unsigned char> vchType;
extern std::map<vchType, uint256> mapMyNames;
extern std::map<vchType, uint256> mapMyMessages;
extern std::map<vchType, std::set<uint256> > mapNamePending;
extern std::set<vchType> setNewHashes;

class CHooks;

unsigned char GetAddressVersion();

std::vector<unsigned char> vchFromString(const std::string &str);
string stringFromVch(const vector<unsigned char> &vch);
int IndexOfNameOutput(const CTransaction& tx);
bool GetValueOfNameTx(const CTransaction& tx, std::vector<unsigned char>& value);
bool DecodeMessageTx(const CTransaction& tx, int& op, int& nOut, std::vector<std::vector<unsigned char> >& vvch);
bool DecodeNameTx(const CTransaction& tx, int& op, int& nOut, std::vector<std::vector<unsigned char> >& vvch);
bool DecodeNameScript(const CScript& script, int& op, std::vector<std::vector<unsigned char> > &vvch, CScript::const_iterator& pc);
bool DecodeNameScript(const CScript& script, int& op, std::vector<std::vector<unsigned char> > &vvch);

/* Handle the name operation part of the RPC call createrawtransaction.  */
class CHooks
{
public:
    virtual bool ConnectInputs(
      std::map<uint256, CTxIndex>& mapTestPool,
      const CTransaction& tx,
      std::vector<CTransaction>& vTxPrev,
      std::vector<CTxIndex>& vTxindex,
      CBlockIndex* pindexBlock,
      CDiskTxPos& txPos,
      bool fBlock,
      bool fMiner
    ) = 0;

    virtual bool AcceptToMemoryPool(const CTransaction& tx) = 0;
    virtual void RemoveFromMemoryPool(const CTransaction& tx) = 0;

    /* These are for display and wallet management purposes.  Not for use to decide
     * whether to spend a coin. */
    virtual bool IsMine(const CTransaction& tx) = 0;
    virtual bool IsMine(const CTransaction& tx, const CTxOut& txout, bool ignore_name_new = false) = 0;
};

extern CHooks* InitHook();

#endif
