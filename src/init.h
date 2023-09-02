#ifndef BITCOIN_INIT_H
#define BITCOIN_INIT_H 

#include "wallet.h"

extern __wx__* pwalletMainId;
void StartShutdown();
void Shutdown(void* parg);
bool AppInit2();
std::string HelpMessage();

#endif
