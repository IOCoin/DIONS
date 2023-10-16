#ifndef BITCOIN_VERSION_H
#define BITCOIN_VERSION_H 

#include "ClientVersion.h"
#include <string>





static const int CLIENT_VERSION =
  3000000 * CLIENT_VERSION_MAJOR
  + 10000 * CLIENT_VERSION_MINOR
  + 100 * CLIENT_VERSION_REVISION
  + 0 * CLIENT_VERSION_BUILD;

extern const std::string CLIENT_NAME;
extern const std::string CLIENT_BUILD;
extern const std::string CLIENT_DATE;




static const int DATABASE_VERSION = 70509;





static const int PROTOCOL_VERSION = 60022;
static const int X4_PROTOCOL_VERSION = 60021;
static const int X3_PROTOCOL_VERSION = 60020;
static const int X2_PROTOCOL_VERSION = 60019;
static const int X1_PROTOCOL_VERSION = 60017;


static const int INIT_PROTO_VERSION = 209;


static const int MIN_PEER_PROTO_VERSION = PROTOCOL_VERSION;



static const int CADDR_TIME_VERSION = 31402;


static const int NOBLKS_VERSION_START = 60002;
static const int NOBLKS_VERSION_END = 60006;


static const int BIP0031_VERSION = 60000;


static const int MEMPOOL_GD_VERSION = 60002;

#endif
