#ifndef BITCOIN_NETBASE_H
#define BITCOIN_NETBASE_H 

#include <string>
#include <vector>

#include "Serialize.h"
#include "Compat.h"

extern int nConnectTimeout;
extern bool fNameLookup;

#ifdef WIN32

#undef SetPort
#endif

enum Network
{
    NET_UNROUTABLE,
    NET_IPV4,
    NET_IPV6,
    NET_TOR,
    NET_I2P,

    NET_MAX,
};


class CNetAddr
{
    protected:
        unsigned char ip[16];

    public:
        CNetAddr();
        CNetAddr(const struct in_addr& ipv4Addr);
        explicit CNetAddr(const char *pszIp, bool fAllowLookup = false);
        explicit CNetAddr(const std::string &strIp, bool fAllowLookup = false);
        void Init();
        void SetIP(const CNetAddr& ip);
        bool SetSpecial(const std::string &aliasStr);
        bool IsIPv4() const;
        bool IsIPv6() const;
        bool IsRFC1918() const;
        bool IsRFC3849() const;
        bool IsRFC3927() const;
        bool IsRFC3964() const;
        bool IsRFC4193() const;
        bool IsRFC4380() const;
        bool IsRFC4843() const;
        bool IsRFC4862() const;
        bool IsRFC6052() const;
        bool IsRFC6145() const;
        bool IsTor() const;
        bool IsI2P() const;
        bool IsLocal() const;
        bool IsRoutable() const;
        bool IsValid() const;
        bool IsMulticast() const;
        enum Network GetNetwork() const;
        std::string ToString() const;
        std::string ToStringIP() const;
        unsigned int GetByte(int n) const;
        uint64_t GetHash() const;
        bool GetInAddr(struct in_addr* pipv4Addr) const;
        std::vector<unsigned char> GetGroup() const;
        int GetReachabilityFrom(const CNetAddr *paddrPartner = NULL) const;
        void print() const;

        CNetAddr(const struct in6_addr& pipv6Addr);
        bool GetIn6Addr(struct in6_addr* pipv6Addr) const;

        friend bool operator==(const CNetAddr& a, const CNetAddr& b);
        friend bool operator!=(const CNetAddr& a, const CNetAddr& b);
        friend bool operator<(const CNetAddr& a, const CNetAddr& b);

        IMPLEMENT_SERIALIZE
            (
             READWRITE(FLATDATA(ip));
            )
};


class CService : public CNetAddr
{
    protected:
        unsigned short port;

    public:
        CService();
        CService(const CNetAddr& ip, unsigned short port);
        CService(const struct in_addr& ipv4Addr, unsigned short port);
        CService(const struct sockaddr_in& addr);
        explicit CService(const char *pszIpPort, int portDefault, bool fAllowLookup = false);
        explicit CService(const char *pszIpPort, bool fAllowLookup = false);
        explicit CService(const std::string& strIpPort, int portDefault, bool fAllowLookup = false);
        explicit CService(const std::string& strIpPort, bool fAllowLookup = false);
        void Init();
        void SetPort(unsigned short portIn);
        unsigned short GetPort() const;
        bool GetSockAddr(struct sockaddr* paddr, socklen_t *addrlen) const;
        bool SetSockAddr(const struct sockaddr* paddr);
        friend bool operator==(const CService& a, const CService& b);
        friend bool operator!=(const CService& a, const CService& b);
        friend bool operator<(const CService& a, const CService& b);
        std::vector<unsigned char> GetKey() const;
        std::string ToString() const;
        std::string ToStringPort() const;
        std::string ToStringIPPort() const;
        void print() const;

        CService(const struct in6_addr& ipv6Addr, unsigned short port);
        CService(const struct sockaddr_in6& addr);

        IMPLEMENT_SERIALIZE
            (
             CService* pthis = const_cast<CService*>(this);
             READWRITE(FLATDATA(ip));
             unsigned short portN = htons(port);
             READWRITE(portN);
             if (fRead)
                 pthis->port = ntohs(portN);
            )
};

typedef std::pair<CService, int> proxyType;

enum Network ParseNetwork(std::string net);
void SplitHostPort(std::string in, int &portOut, std::string &hostOut);
bool SetProxy(enum Network net, CService addrProxy, int nSocksVersion = 5);
bool GetProxy(enum Network net, proxyType &proxyInfoOut);
bool IsProxy(const CNetAddr &addr);
bool SetNameProxy(CService addrProxy, int nSocksVersion = 5);
bool HaveNameProxy();
bool LookupHost(const char *pszName, std::vector<CNetAddr>& vIP, unsigned int nMaxSolutions = 0, bool fAllowLookup = true);
bool Lookup(const char *pszName, CService& addr, int portDefault = 0, bool fAllowLookup = true);
bool Lookup(const char *pszName, std::vector<CService>& vAddr, int portDefault = 0, bool fAllowLookup = true, unsigned int nMaxSolutions = 0);
bool LookupNumeric(const char *pszName, CService& addr, int portDefault = 0);
bool ConnectSocket(const CService &addr, SOCKET& hSocketRet, int nTimeout = nConnectTimeout);
bool ConnectSocketByName(CService &addr, SOCKET& hSocketRet, const char *pszDest, int portDefault = 0, int nTimeout = nConnectTimeout);

#endif
