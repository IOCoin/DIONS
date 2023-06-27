#ifndef __cplusplus
# error This header can only be compiled as C++.
#endif

#ifndef __INCLUDED_PROTOCOL_H__
#define __INCLUDED_PROTOCOL_H__ 

#include "serialize.h"
#include "netbase.h"
#include <string>
#include "uint256.h"

extern bool fTestNet;
static inline unsigned short GetDefaultPort(const bool testnet = fTestNet)
{
    return testnet ? 1901 : 33764;
}


extern unsigned char pchMessageStart[4];







class CMessageHeader
{
    public:
        CMessageHeader();
        CMessageHeader(const char* pszCommand, unsigned int nMessageSizeIn);

        std::string GetCommand() const;
        bool IsValid() const;

        IMPLEMENT_SERIALIZE
            (
             READWRITE(FLATDATA(pchMessageStart));
             READWRITE(FLATDATA(pchCommand));
             READWRITE(nMessageSize);
             READWRITE(nChecksum);
            )


    public:
        enum {
            MESSAGE_START_SIZE=sizeof(::pchMessageStart),
            COMMAND_SIZE=12,
            MESSAGE_SIZE_SIZE=sizeof(int),
            CHECKSUM_SIZE=sizeof(int),

            MESSAGE_SIZE_OFFSET=MESSAGE_START_SIZE+COMMAND_SIZE,
            CHECKSUM_OFFSET=MESSAGE_SIZE_OFFSET+MESSAGE_SIZE_SIZE,
            HEADER_SIZE=MESSAGE_START_SIZE+COMMAND_SIZE+MESSAGE_SIZE_SIZE+CHECKSUM_SIZE
        };
        char pchMessageStart[MESSAGE_START_SIZE];
        char pchCommand[COMMAND_SIZE];
        unsigned int nMessageSize;
        unsigned int nChecksum;
};


enum
{
    NODE_NETWORK = (1 << 0),
};


class CAddress : public CService
{
    public:
        CAddress();
        explicit CAddress(CService ipIn, uint64_t nServicesIn=NODE_NETWORK);

        void Init();

        IMPLEMENT_SERIALIZE
            (
             CAddress* pthis = const_cast<CAddress*>(this);
             CService* pip = (CService*)pthis;
             if (fRead)
                 pthis->Init();
             if (nType & SER_DISK)
                 READWRITE(nVersion);
             if ((nType & SER_DISK) ||
                 (nVersion >= CADDR_TIME_VERSION && !(nType & SER_GETHASH)))
                 READWRITE(nTime);
             READWRITE(nServices);
             READWRITE(*pip);
            )

        void print() const;


    public:
        uint64_t nServices;


        unsigned int nTime;


        int64_t nLastTry;
};


class CInv
{
    public:
        CInv();
        CInv(int typeIn, const uint256& hashIn);
        CInv(const std::string& strType, const uint256& hashIn);

        IMPLEMENT_SERIALIZE
        (
            READWRITE(type);
            READWRITE(hash);
        )

        friend bool operator<(const CInv& a, const CInv& b);

        bool IsKnownType() const;
        const char* GetCommand() const;
        std::string ToString() const;
        void print() const;


    public:
        int type;
        uint256 hash;
};

#endif
