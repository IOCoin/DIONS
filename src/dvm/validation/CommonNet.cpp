// Licensed under the GNU General Public License, Version 3.


#include "CommonNet.h"
using namespace std;
using namespace dev;
using namespace dev::dvm;

namespace dev
{
namespace dvm
{
#pragma GCC diagnostic ignored "-Wunused-variable"
namespace
{
char dummy;
}


char const* dvmPacketTypeToString(EthSubprotocolPacketType _packetType)
{
    switch (_packetType)
    {
    case StatusPacket:
        return "Status";
    case NewBlockHashesPacket:
        return "NewBlockHashes";
    case TransactionsPacket:
        return "Transactions";
    case GetBlockHeadersPacket:
        return "GetBlockHeaders";
    case BlockHeadersPacket:
        return "BlockHeaders";
    case GetBlockBodiesPacket:
        return "GetBlockBodies";
    case BlockBodiesPacket:
        return "BlockBodies";
    case NewBlockPacket:
        return "NewBlock";
    case GetNodeDataPacket:
        return "GetNodeData";
    case NodeDataPacket:
        return "NodeData";
    case GetReceiptsPacket:
        return "GetReceipts";
    case ReceiptsPacket:
        return "Receipts";
    default:
        return "UnknownEthPacket";
    }
}
}  // namespace dvm
}  // namespace dev
