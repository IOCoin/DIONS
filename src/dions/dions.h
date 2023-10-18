#pragma once

#include "json/json_spirit.h"

#include "ReactorRelay.h"
#include "ccoin/Transaction.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"
#include "rpc/client.h"
#include "rpc/dions_face.h"

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
static const int OP_VERTEX = 0x09;
static const int OP_MAP_PROJECT = 0x10;
static const int MIN_SET_DEPTH = 1;

static const int UI_MAX_XUNIT_LENGTH = 520;

extern std::map<vchType, uint256> mapLocator;
extern std::map<vchType, uint256> mapMyMessages;
extern std::map<vchType, std::set<uint256> > mapState;
extern std::set<vchType> setNewHashes;


class cba;

int checkAddress(string addr, cba& a);

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
bool IsMinePost(const CTransaction& tx, const CTxOut& txout,bool ignore_registerAlias = false);


class Dions : public DionsFace
{
public:
  Dions()
  {
  }
  virtual json_spirit::Value aliasList(const json_spirit::Array&, bool) override;
  virtual json_spirit::Value vertexScan(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value gw1(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value myRSAKeys(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value myRSAKeys__(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value publicKeyExports(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value publicKeys(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value decryptedMessageList(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value plainTextMessageList(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value externFrame__(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value internFrame__(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value aliasOut(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value nodeValidate(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value validateLocator(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value nodeRetrieve(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value getNodeRecord(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value aliasList__(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value nodeDebug(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value nodeDebug1(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value transform(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value primaryCXValidate(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value vextract(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value vEPID(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value validate(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value transientStatus__C(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value updateEncryptedAliasFile(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value updateEncryptedAlias(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value decryptAlias(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value transferEncryptedExtPredicate(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value transferEncryptedAlias(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value transferAlias(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value uC(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value transientStatus__(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value updateAliasFile(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value updateAlias_executeContractPayload(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value updateAlias(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value publicKey(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value sendSymmetric(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value sendPublicKey(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value sendPlainMessage(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value sendMessage(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value registerAliasGenerate(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value registerAlias(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value alias(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value statusList(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value updateEncrypt(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value downloadDecrypt(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value downloadDecryptEPID(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value ioget(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value extract(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value vtx(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value mapVertex(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value vtxtrace(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value mapProject(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value projection(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value xstat(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value svtx(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value simplexU(const json_spirit::Array&,bool) override;
  virtual json_spirit::Value psimplex(const json_spirit::Array&,bool) override;
};

