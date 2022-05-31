/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * 
 * Licensed under the Apache License, Version 2.0.
 */

#include <ethash/ethash.h>

char test(void);

char test()
{
    char sum = 0;
    sum += ETHASH_EPOCH_LENGTH;
    sum += ETHASH_LIGHT_CACHE_ITEM_SIZE;
    sum += ETHASH_FULL_DATASET_ITEM_SIZE;
    return sum;
}
