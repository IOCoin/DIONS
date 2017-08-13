// Copyright (c) 2017 IODigital foundation developers 
#ifndef DIONS_H
#define DIONS_H 

#include "json/json_spirit.h"

#include "reactor_relay.h"

static const int64_t CTRL__ = 0;

static const int MAX_LOCATOR_LENGTH = 684;

static const int MAX_XUNIT_LENGTH = 1024 * 1024;
static const int OP_ALIAS_INVALID = 0x00;
static const int OP_ALIAS_ENCRYPTED = 0x01;
static const int OP_ALIAS_SET = 0x02;
static const int OP_ALIAS_RELAY = 0x03;
static const int OP_ALIAS_NOP = 0x04;
static const int OP_MESSAGE = 0x05;
static const int OP_ENCRYPTED_MESSAGE = 0x06;
static const int OP_PUBLIC_KEY = 0x07;
static const int OP_ALIAS_RELAY_ENCRYPTED = 0x08;
static const int MIN_SET_DEPTH = 1;

static const int UI_MAX_XUNIT_LENGTH = 520;

extern std::map<vchType, uint256> mapLocator;
extern std::map<vchType, uint256> mapMyMessages;
extern std::map<vchType, std::set<uint256> > mapState;
extern std::set<vchType> setNewHashes;


unsigned char GetAddressVersion();
class CBitcoinAddress;

int checkAddress(string addr, CBitcoinAddress& a);

std::vector<unsigned char> vchFromString(const std::string &str);
string stringFromVch(const vector<unsigned char> &vch);
int aliasOutIndex(const CTransaction& tx);
bool aliasTxValue(const CTransaction& tx, std::vector<unsigned char>& value);
bool mTx(const CTransaction& tx, int& op, int& nOut, std::vector<std::vector<unsigned char> >& vvch);
bool aliasTx(const CTransaction& tx, int& op, int& nOut, std::vector<std::vector<unsigned char> >& vvch);
bool aliasScript(const CScript& script, int& op, std::vector<std::vector<unsigned char> > &vvch, CScript::const_iterator& pc);
bool aliasScript(const CScript& script, int& op, std::vector<std::vector<unsigned char> > &vvch);
          bool ConnectInputsPost( map<uint256, CTxIndex>& mapTestPool,
           const CTransaction& tx,
           vector<CTransaction>& vTxPrev,
           vector<CTxIndex>& vTxindex,
           CBlockIndex* pindexBlock,
           CDiskTxPos& txPos,
           bool fBlock,
           bool fMiner);
      bool AcceptToMemoryPoolPost(const CTransaction& tx);
      void RemoveFromMemoryPoolPost(const CTransaction& tx);
     bool IsMinePost(const CTransaction& tx);
     bool IsMinePost(const CTransaction& tx, const CTxOut& txout, bool ignore_registerAlias = false);

#endif
