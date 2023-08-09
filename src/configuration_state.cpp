#include "configuration_state.h"

  const unsigned int ConfigurationState::MAX_ORPHAN_TRANSACTIONS = CBlock::MAX_BLOCK_SIZE/100;
  const unsigned int ConfigurationState::MAX_INV_SZ = 50000;
  const unsigned int ConfigurationState::MAX_BLOCK_SIGOPS = CBlock::MAX_BLOCK_SIZE/50;
  const unsigned int ConfigurationState::MAX_STANDARD_TX_SIZE = 0.6 * CBlock::MAX_BLOCK_SIZE_GEN;
  const int ConfigurationState::LAST_POW_BLOCK = 12815;
  const int64_t ConfigurationState::MAX_MONEY = std::numeric_limits<int64_t>::max();
  const unsigned int ConfigurationState::LOCKTIME_THRESHOLD = 500000000;
  const unsigned int ConfigurationState::MAX_TX_INFO_LEN = 140;
  const int64_t ConfigurationState::MIN_COIN_YEAR_REWARD = 1 * CENT;
  const int64_t ConfigurationState::MAX_COIN_YEAR_REWARD = 3 * CENT;
  const int ConfigurationState::SHADE_FEATURE_UPDATE = 75 * 500 + 1860837;
  const int ConfigurationState::BASELINE_LOCK = 0x00ff0;
  const int ConfigurationState::BLOCK_REWARD_HALVING = 0x2dc6c0;

  int ConfigurationState::nStakeMinConfirmations = 500;

int64_t ConfigurationState::trace() 
{  
  return MIN_COIN_YEAR_REWARD ;
}
