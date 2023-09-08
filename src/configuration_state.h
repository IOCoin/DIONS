#ifndef CONFIGURATION_STATE_H
#define CONFIGURATION_STATE_H

#include <cstdint>
#include "block.h"
#include "wallet/transaction.h"
#include "util.h"

class ConfigurationState
{
public:
  static const unsigned int MAX_ORPHAN_TRANSACTIONS;
  static const unsigned int MAX_INV_SZ ;
  static const unsigned int MAX_BLOCK_SIGOPS ;
  static const unsigned int MAX_STANDARD_TX_SIZE ;
  static const int LAST_POW_BLOCK ;
  static const int64_t MAX_MONEY ;
  static const unsigned int LOCKTIME_THRESHOLD ;
  static const unsigned int MAX_TX_INFO_LEN ;
  static const int64_t MIN_COIN_YEAR_REWARD ;
  static const int64_t MAX_COIN_YEAR_REWARD ;
  static const int SHADE_FEATURE_UPDATE ;
  static const int BASELINE_LOCK ;
  static const int BLOCK_REWARD_HALVING ;
  static int nStakeMinConfirmations;

  int64_t trace();

  int64_t nTransactionFee = CTransaction::MIN_TX_FEE;
};

#endif
