// Aleth: Ethereum C++ client, tools and libraries.
// Copyright 2014-2019 Aleth Authors.
// Licensed under the GNU General Public License, Version 3.

#include "EthereumCapability.h"
#include "BlockChain.h"
#include "BlockChainSync.h"
#include "BlockQueue.h"
#include "TransactionQueue.h"
#include <libethcore/Exceptions.h>
#include <libp2p/Common.h>
#include <libp2p/Host.h>
#include <libp2p/Session.h>
#include <chrono>
#include <thread>

using namespace std;
using namespace dev;
using namespace dev::eth;

char const* const EthereumCapability::c_stateNames[static_cast<int>(SyncState::Size)] = {
    "NotSynced", "Idle", "Waiting", "Blocks", "State"};

std::chrono::milliseconds constexpr EthereumCapability::c_backgroundWorkInterval;

namespace
{
constexpr unsigned c_maxSendTransactionsCount = 256;
constexpr unsigned c_maxSendHeadersCount = 1024;
constexpr unsigned c_maxIncomingNewHashesCount = 1024;
constexpr unsigned c_peerTimeoutSeconds = 10;
constexpr int c_minBlockBroadcastPeers = 4;
constexpr size_t c_maxSendNewBlocksCount = 20;

string toString(Asking _a)
{
    switch (_a)
    {
    case Asking::BlockHeaders:
        return "BlockHeaders";
    case Asking::BlockBodies:
        return "BlockBodies";
    case Asking::NodeData:
        return "NodeData";
    case Asking::Receipts:
        return "Receipts";
    case Asking::Nothing:
        return "Nothing";
    case Asking::State:
        return "State";
    case Asking::WarpManifest:
        return "WarpManifest";
    case Asking::WarpData:
        return "WarpData";
    }
    return "?";
}

class EthereumPeerObserver : public EthereumPeerObserverFace
{
public:
    EthereumPeerObserver(shared_ptr<BlockChainSync> _sync, TransactionQueue& _tq)
      : m_sync(_sync), m_tq(_tq)
    {}

    void onPeerStatus(EthereumPeer const& _peer) override
    {
        try
        {
            m_sync->onPeerStatus(_peer);
        }
        catch (FailedInvariant const&)
        {
            // "fix" for https://github.com/ethereum/webthree-umbrella/issues/300
            cwarn << "Failed invariant during sync, restarting sync";
            m_sync->restartSync();
        }
    }

    void onPeerTransactions(NodeID const& _peerID, RLP const& _r) override
    {
        unsigned itemCount = _r.itemCount();
        LOG(m_logger) << "Transactions (" << dec << itemCount << " entries)";
        m_tq.enqueue(_r, _peerID);
    }

    void onPeerAborting() override
    {
        try
        {
            m_sync->onPeerAborting();
        }
        catch (Exception&)
        {
            cwarn << "Exception on peer destruction: "
                  << boost::current_exception_diagnostic_information();
        }
    }

    void onPeerBlockHeaders(NodeID const& _peerID, RLP const& _headers) override
    {
        try
        {
            m_sync->onPeerBlockHeaders(_peerID, _headers);
        }
        catch (FailedInvariant const&)
        {
            // "fix" for https://github.com/ethereum/webthree-umbrella/issues/300
            cwarn << "Failed invariant during sync, restarting sync";
            m_sync->restartSync();
        }
    }

    void onPeerBlockBodies(NodeID const& _peerID, RLP const& _r) override
    {
        try
        {
            m_sync->onPeerBlockBodies(_peerID, _r);
        }
        catch (FailedInvariant const&)
        {
            // "fix" for https://github.com/ethereum/webthree-umbrella/issues/300
            cwarn << "Failed invariant during sync, restarting sync";
            m_sync->restartSync();
        }
    }

    void onPeerNewHashes(
        NodeID const& _peerID, std::vector<std::pair<h256, u256>> const& _hashes) override
    {
        try
        {
            m_sync->onPeerNewHashes(_peerID, _hashes);
        }
        catch (FailedInvariant const&)
        {
            // "fix" for https://github.com/ethereum/webthree-umbrella/issues/300
            cwarn << "Failed invariant during sync, restarting sync";
            m_sync->restartSync();
        }
    }

    void onPeerNewBlock(NodeID const& _peerID, RLP const& _r) override
    {
        try
        {
            m_sync->onPeerNewBlock(_peerID, _r);
        }
        catch (FailedInvariant const&)
        {
            // "fix" for https://github.com/ethereum/webthree-umbrella/issues/300
            cwarn << "Failed invariant during sync, restarting sync";
            m_sync->restartSync();
        }
    }

    void onPeerNodeData(NodeID const& /* _peerID */, RLP const& _r) override
    {
        unsigned itemCount = _r.itemCount();
        LOG(m_logger) << "Node Data (" << dec << itemCount << " entries)";
    }

    void onPeerReceipts(NodeID const& /* _peerID */, RLP const& _r) override
    {
        unsigned itemCount = _r.itemCount();
        LOG(m_logger) << "Receipts (" << dec << itemCount << " entries)";
    }

private:
    shared_ptr<BlockChainSync> m_sync;
    TransactionQueue& m_tq;

    Logger m_logger{createLogger(VerbosityDebug, "host")};
};

class EthereumHostData : public EthereumHostDataFace
{
public:
    EthereumHostData(BlockChain const& _chain, OverlayDB const& _db) : m_chain(_chain), m_db(_db) {}

    pair<bytes, unsigned> blockHeaders(
        RLP const& _blockId, unsigned _maxHeaders, u256 _skip, bool _reverse) const override
    {
        auto numHeadersToSend = _maxHeaders;

        auto step = static_cast<unsigned>(_skip) + 1;
        assert(step > 0 && "step must not be 0");

        h256 blockHash;
        if (_blockId.size() == 32)  // block id is a hash
        {
            blockHash = _blockId.toHash<h256>();
            cnetlog << "GetBlockHeaders (block (hash): " << blockHash
                    << ", maxHeaders: " << _maxHeaders << ", skip: " << _skip
                    << ", reverse: " << _reverse << ")";

            if (!m_chain.isKnown(blockHash))
                blockHash = {};
            else if (!_reverse)
            {
                auto n = m_chain.number(blockHash);
                if (numHeadersToSend == 0)
                    blockHash = {};
                else if (n != 0 || blockHash == m_chain.genesisHash())
                {
                    auto top = n + uint64_t(step) * numHeadersToSend - 1;
                    auto lastBlock = m_chain.number();
                    if (top > lastBlock)
                    {
                        numHeadersToSend = (lastBlock - n) / step + 1;
                        top = n + step * (numHeadersToSend - 1);
                    }
                    assert(top <= lastBlock && "invalid top block calculated");
                    // override start block hash with the hash of the top block we have
                    blockHash = m_chain.numberHash(static_cast<unsigned>(top));
                }
                else
                    blockHash = {};
            }
        }
        else // block id is a number
        {
            auto n = _blockId.toInt<bigint>();
            cnetlog << "GetBlockHeaders (" << n << " max: " << _maxHeaders << " skip: " << _skip
                    << (_reverse ? " reverse" : "") << ")";

            if (!_reverse)
            {
                auto lastBlock = m_chain.number();
                if (n > lastBlock || numHeadersToSend == 0)
                    blockHash = {};
                else
                {
                    bigint top = n + uint64_t(step) * (numHeadersToSend - 1);
                    if (top > lastBlock)
                    {
                        numHeadersToSend = (lastBlock - static_cast<unsigned>(n)) / step + 1;
                        top = n + step * (numHeadersToSend - 1);
                    }
                    assert(top <= lastBlock && "invalid top block calculated");
                    // override start block hash with the hash of the top block we have
                    blockHash = m_chain.numberHash(static_cast<unsigned>(top));
                }
            }
            else if (n <= std::numeric_limits<unsigned>::max())
                blockHash = m_chain.numberHash(static_cast<unsigned>(n));
            else
                blockHash = {};
        }

        auto nextHash = [this](h256 _h, unsigned _step) {
            constexpr unsigned c_blockNumberUsageLimit = 1000;

            const auto lastBlock = m_chain.number();
            // find the number of the block below which we don't expect BC changes.
            const auto limitBlock =
                lastBlock > c_blockNumberUsageLimit ?
                    lastBlock - c_blockNumberUsageLimit :
                    0;

            while (_step) // parent hash traversal
            {
                auto details = m_chain.details(_h);
                if (details.number < limitBlock)
                    // stop using parent hash traversal, fallback to using block numbers
                    break;
                _h = details.parentHash;
                --_step;
            }

            if (_step) // still need lower block
            {
                auto n = m_chain.number(_h);
                if (n >= _step)
                    _h = m_chain.numberHash(n - _step);
                else
                    _h = {};
            }


            return _h;
        };

        bytes rlp;
        unsigned itemCount = 0;
        vector<h256> hashes;
        for (unsigned i = 0; i != numHeadersToSend; ++i)
        {
            if (!blockHash || !m_chain.isKnown(blockHash))
                break;

            hashes.push_back(blockHash);
            ++itemCount;

            blockHash = nextHash(blockHash, step);
        }

        for (unsigned i = 0; i < hashes.size() && rlp.size() < c_maxPayload; ++i)
            rlp += m_chain.headerData(hashes[_reverse ? i : hashes.size() - 1 - i]);

        return make_pair(rlp, itemCount);
    }

    pair<bytes, unsigned> blockBodies(RLP const& _blockHashes) const override
    {
        unsigned const count = static_cast<unsigned>(_blockHashes.itemCount());

        bytes rlp;
        unsigned n = 0;
        auto numBodiesToSend = std::min(count, c_maxBlocks);
        for (unsigned i = 0; i < numBodiesToSend && rlp.size() < c_maxPayload; ++i)
        {
            auto h = _blockHashes[i].toHash<h256>();
            if (m_chain.isKnown(h))
            {
                bytes blockBytes = m_chain.block(h);
                RLP block{blockBytes};
                RLPStream body;
                body.appendList(2);
                body.appendRaw(block[1].data());  // transactions
                body.appendRaw(block[2].data());  // uncles
                auto bodyBytes = body.out();
                rlp.insert(rlp.end(), bodyBytes.begin(), bodyBytes.end());
                ++n;
            }
        }
        if (count > 20 && n == 0)
            cnetlog << "all " << count << " unknown blocks requested; peer on different chain?";
        else
            cnetlog << n << " blocks known and returned; " << (numBodiesToSend - n)
                    << " blocks unknown; " << (count > c_maxBlocks ? count - c_maxBlocks : 0)
                    << " blocks ignored";

        return make_pair(rlp, n);
    }

    strings nodeData(RLP const& _dataHashes) const override
    {
        unsigned const count = static_cast<unsigned>(_dataHashes.itemCount());

        strings data;
        size_t payloadSize = 0;
        auto numItemsToSend = std::min(count, c_maxNodes);
        for (unsigned i = 0; i < numItemsToSend && payloadSize < c_maxPayload; ++i)
        {
            auto h = _dataHashes[i].toHash<h256>();
            auto node = m_db.lookup(h);
            if (!node.empty())
            {
                payloadSize += node.length();
                data.push_back(move(node));
            }
        }
        cnetlog << data.size() << " nodes known and returned; " << (numItemsToSend - data.size())
                << " unknown; " << (count > c_maxNodes ? count - c_maxNodes : 0) << " ignored";

        return data;
    }

    pair<bytes, unsigned> receipts(RLP const& _blockHashes) const override
    {
        unsigned const count = static_cast<unsigned>(_blockHashes.itemCount());

        bytes rlp;
        unsigned n = 0;
        auto numItemsToSend = std::min(count, c_maxReceipts);
        for (unsigned i = 0; i < numItemsToSend && rlp.size() < c_maxPayload; ++i)
        {
            auto h = _blockHashes[i].toHash<h256>();
            if (m_chain.isKnown(h))
            {
                auto const receipts = m_chain.receipts(h);
                auto receiptsRlpList = receipts.rlp();
                rlp.insert(rlp.end(), receiptsRlpList.begin(), receiptsRlpList.end());
                ++n;
            }
        }
        cnetlog << n << " receipt lists known and returned; " << (numItemsToSend - n)
                << " unknown; " << (count > c_maxReceipts ? count - c_maxReceipts : 0)
                << " ignored";

        return make_pair(rlp, n);
    }

private:
    BlockChain const& m_chain;
    OverlayDB const& m_db;
};
}  // namespace

EthereumCapability::EthereumCapability(shared_ptr<p2p::CapabilityHostFace> _host,
    BlockChain const& _ch, OverlayDB const& _db, TransactionQueue& _tq, BlockQueue& _bq,
    u256 _networkId)
  : m_host(move(_host)),
    m_chain(_ch),
    m_db(_db),
    m_tq(_tq),
    m_bq(_bq),
    m_networkId(_networkId),
    m_hostData(new EthereumHostData(m_chain, m_db))
{
    // TODO: Composition would be better. Left like that to avoid initialization
    //       issues as BlockChainSync accesses other EthereumHost members.
    m_sync.reset(new BlockChainSync(*this));
    m_peerObserver.reset(new EthereumPeerObserver(m_sync, m_tq));
    m_latestBlockHashSent = _ch.currentHash();
    m_latestBlockSent = _ch.currentHash();
    m_tq.onImport([this](ImportResult _ir, h256 const& _h, h512 const& _nodeId) { onTransactionImported(_ir, _h, _nodeId); });
    std::random_device seed;
    m_urng = std::mt19937_64(seed());
}

std::chrono::milliseconds EthereumCapability::backgroundWorkInterval() const
{
    return c_backgroundWorkInterval;
}

bool EthereumCapability::ensureInitialised()
{
    if (!m_latestBlockHashSent)
    {
        assert(!m_latestBlockSent);
        // First time - just initialise.
        m_latestBlockHashSent = m_chain.currentHash();
        m_latestBlockSent = m_chain.currentHash();
        LOG(m_logger) << "Initialising: latest=" << m_latestBlockHashSent;

        m_transactionsSent = m_tq.knownTransactions();
        return true;
    }
    return false;
}

void EthereumCapability::reset()
{
    m_sync->abortSync();

    // reset() can be called from RPC handling thread,
    // but we access m_latestBlockHashSent, m_latestBlockSent, and m_transactionsSent only from the
    // network thread
    m_host->postWork([this]() {
        m_latestBlockHashSent = {};
        m_latestBlockSent = {};
        m_transactionsSent.clear();
    });
}

void EthereumCapability::completeSync()
{
    m_sync->completeSync();
}

void EthereumCapability::maintainTransactions()
{
    // Send any new transactions.
    unordered_map<NodeID, std::vector<size_t>> peerTransactions;
    auto ts = m_tq.topTransactions(c_maxSendTransactionsCount);
    {
        for (size_t i = 0; i < ts.size(); ++i)
        {
            auto const& t = ts[i];
            bool unsent = !m_transactionsSent.count(t.sha3());

            // Build list of peers to send transactions to
            auto const peers = selectPeers([&](EthereumPeer const& _peer) {
                return _peer.isWaitingForTransactions() ||
                       (unsent && !_peer.isTransactionKnown(t.sha3()));
            });
            for (auto const& p: peers)
                peerTransactions[p].push_back(i);
        }
        for (auto const& t: ts)
            m_transactionsSent.insert(t.sha3());
    }

    // Send transactions to peers
    for (auto& peer : m_peers)
    {
        bytes b;
        unsigned n = 0;
        for (auto const& i : peerTransactions[peer.first])
        {
            peer.second.markTransactionAsKnown(ts[i].sha3());
            b += ts[i].rlp();
            ++n;
        }

        if (n || peer.second.isWaitingForTransactions())
        {
            RLPStream ts;
            m_host->prep(peer.first, name(), ts, TransactionsPacket, n).appendRaw(b, n);
            m_host->sealAndSend(peer.first, ts);
            LOG(m_logger) << "Sent " << n << " transactions to " << peer.first;
        }
        peer.second.setWaitingForTransactions(false);
    }
}

vector<NodeID> EthereumCapability::selectPeers(
    std::function<bool(EthereumPeer const&)> const& _predicate) const
{
    vector<NodeID> allowed;
    for (auto const& peer : m_peers)
    {
        if (_predicate(peer.second))
            allowed.push_back(peer.first);
    }
    return allowed;
}

std::vector<NodeID> EthereumCapability::randomPeers(
    std::vector<NodeID> const& _peers, size_t _count) const
{
    std::vector<NodeID> peers{_peers};
    if (peers.empty() || _count >= _peers.size())
        return peers;

    std::shuffle(peers.begin(), peers.end(), m_urng);
    return {peers.begin(), peers.begin() + _count};
}

void EthereumCapability::maintainBlockHashes(h256 const& _currentHash)
{
    // Send any new block hashes
    auto const detailsFrom = m_chain.details(m_latestBlockHashSent);
    auto const detailsTo = m_chain.details(_currentHash);

    if (detailsFrom.totalDifficulty >= detailsTo.totalDifficulty)
        return;
    
    // don't be sending more than c_maxSendNewBlocksCount "new" block hashes. if there are any
    // more we were probably waaaay behind.
    if (diff(detailsFrom.number, detailsTo.number) > c_maxSendNewBlocksCount)
    {
        m_latestBlockHashSent = _currentHash;
        return;
    }

    LOG(m_logger) << "Sending new block hashes (current is " << _currentHash << ", was "
                    << m_latestBlockHashSent << ")";

    h256s const blockHashes =
        std::get<0>(m_chain.treeRoute(m_latestBlockHashSent, _currentHash, false, false, true));

    auto const peersWithoutBlock = selectPeers(
        [&](EthereumPeer const& _peer) { return !_peer.isBlockKnown(_currentHash); });
    for (NodeID const& peerID : peersWithoutBlock)
    {
        RLPStream ts;
        m_host->prep(peerID, name(), ts, NewBlockHashesPacket, blockHashes.size());
        for (auto const& bh : blockHashes)
        {
            ts.appendList(2);
            ts.append(bh);
            ts.append(m_chain.number(bh));
        }

        auto itPeer = m_peers.find(peerID);
        if (itPeer != m_peers.end())
        {
            m_host->sealAndSend(peerID, ts);
            itPeer->second.clearKnownBlocks();
        }
    }
    if (!peersWithoutBlock.empty())
        LOG(m_logger) << "Announced " << blockHashes.size() << " block(s) to "
                        << peersWithoutBlock.size() << " peers";

    m_latestBlockHashSent = _currentHash;
}

bool EthereumCapability::isSyncing() const
{
    return m_sync->isSyncing();
}

SyncStatus EthereumCapability::status() const
{
    return m_sync->status();
}

void EthereumCapability::onTransactionImported(
    ImportResult _ir, h256 const& _h, h512 const& _nodeId)
{
    m_host->postWork([this, _ir, _h, _nodeId]() {
        auto itPeerStatus = m_peers.find(_nodeId);
        if (itPeerStatus == m_peers.end())
            return;

        auto& peer = itPeerStatus->second;

        peer.markTransactionAsKnown(_h);
        switch (_ir)
        {
        case ImportResult::Malformed:
            m_host->updateRating(_nodeId, -100);
            break;
        case ImportResult::AlreadyKnown:
            // if we already had the transaction, then don't bother sending it on.
            m_transactionsSent.insert(_h);
            m_host->updateRating(_nodeId, 0);
            break;
        case ImportResult::Success:
            m_host->updateRating(_nodeId, 100);
            break;
        default:;
        }
    });
}

void EthereumCapability::onConnect(NodeID const& _peerID, u256 const& _peerCapabilityVersion)
{
    m_host->addNote(_peerID, "manners", m_host->isRude(_peerID, name()) ? "RUDE" : "nice");

    EthereumPeer peer{m_host, _peerID, _peerCapabilityVersion};
    m_peers.emplace(_peerID, peer);
    peer.requestStatus(m_networkId, m_chain.details().totalDifficulty, m_chain.currentHash(),
        m_chain.genesisHash());
}

void EthereumCapability::onDisconnect(NodeID const& _peerID)
{
    // TODO lower peer's rating or mark as rude if it disconnects when being asked for something
    m_peerObserver->onPeerAborting();

    m_peers.erase(_peerID);
}

bool EthereumCapability::interpretCapabilityPacket(
    NodeID const& _peerID, unsigned _id, RLP const& _r)
{
    auto& remotePeer = peer(_peerID);
    remotePeer.setLastAsk(std::chrono::system_clock::to_time_t(chrono::system_clock::now()));

    try
    {
        switch (_id)
        {
        case StatusPacket:
        {
            auto const peerProtocolVersion = _r[0].toInt<unsigned>();
            auto const networkId = _r[1].toInt<u256>();
            auto const totalDifficulty = _r[2].toInt<u256>();
            auto const latestHash = _r[3].toHash<h256>();
            auto const genesisHash = _r[4].toHash<h256>();

            LOG(m_logger) << "Status (from " << _peerID << "): " << peerProtocolVersion << " / "
                          << networkId << " / " << genesisHash << ", TD: " << totalDifficulty
                          << " = " << latestHash;

            remotePeer.setStatus(
                peerProtocolVersion, networkId, totalDifficulty, latestHash, genesisHash);
            setIdle(_peerID);
            m_peerObserver->onPeerStatus(remotePeer);
            break;
        }
        case TransactionsPacket:
        {
            m_peerObserver->onPeerTransactions(_peerID, _r);
            break;
        }
        case GetBlockHeadersPacket:
        {
            /// Packet layout:
            /// [ block: { P , B_32 }, maxHeaders: P, skip: P, reverse: P in { 0 , 1 } ]
            const auto blockId = _r[0];
            const auto maxHeaders = _r[1].toInt<u256>();
            const auto skip = _r[2].toInt<u256>();
            const auto reverse = _r[3].toInt<bool>();

            auto const numHeadersToSend = maxHeaders <= c_maxSendHeadersCount ?
                                        static_cast<unsigned>(maxHeaders) :
                                        c_maxSendHeadersCount;

            if (skip > std::numeric_limits<unsigned>::max() - 1)
            {
                LOG(m_loggerDetail)
                    << "Requested block skip is too big: " << skip << " (peer: " << _peerID << ")";
                break;
            }

            pair<bytes, unsigned> const rlpAndItemCount =
                m_hostData->blockHeaders(blockId, numHeadersToSend, skip, reverse);

            RLPStream s;
            m_host->prep(_peerID, name(), s, BlockHeadersPacket, rlpAndItemCount.second)
                .appendRaw(rlpAndItemCount.first, rlpAndItemCount.second);
            m_host->sealAndSend(_peerID, s);
            m_host->updateRating(_peerID, 0);
            break;
        }
        case BlockHeadersPacket:
        {
            if (remotePeer.asking() != Asking::BlockHeaders)
                LOG(m_loggerImpolite) << "Peer " << _peerID
                                      << " giving us block headers when we didn't ask for them.";
            else
            {
                setIdle(_peerID);
                m_peerObserver->onPeerBlockHeaders(_peerID, _r);
            }
            break;
        }
        case GetBlockBodiesPacket:
        {
            unsigned count = static_cast<unsigned>(_r.itemCount());
            LOG(m_logger) << "GetBlockBodies (" << dec << count << " entries) from " << _peerID;

            if (!count)
            {
                LOG(m_loggerImpolite) << "Zero-entry GetBlockBodies: Not replying to " << _peerID;
                m_host->updateRating(_peerID, -10);
                break;
            }

            pair<bytes, unsigned> const rlpAndItemCount = m_hostData->blockBodies(_r);

            m_host->updateRating(_peerID, 0);
            RLPStream s;
            m_host->prep(_peerID, name(), s, BlockBodiesPacket, rlpAndItemCount.second)
                .appendRaw(rlpAndItemCount.first, rlpAndItemCount.second);
            m_host->sealAndSend(_peerID, s);
            break;
        }
        case BlockBodiesPacket:
        {
            if (remotePeer.asking() != Asking::BlockBodies)
                LOG(m_loggerImpolite)
                    << "Peer " << _peerID << " giving us block bodies when we didn't ask for them.";
            else
            {
                setIdle(_peerID);
                m_peerObserver->onPeerBlockBodies(_peerID, _r);
            }
            break;
        }
        case NewBlockPacket:
        {
            m_peerObserver->onPeerNewBlock(_peerID, _r);
            break;
        }
        case NewBlockHashesPacket:
        {
            unsigned itemCount = _r.itemCount();

            LOG(m_logger) << "BlockHashes (" << dec << itemCount << " entries) "
                          << (itemCount ? "" : " : NoMoreHashes") << " from " << _peerID;

            if (itemCount > c_maxIncomingNewHashesCount)
            {
                LOG(m_logger) << "Received too many hashes (" << itemCount << ") from " << _peerID
                              << ", only processing first " << c_maxIncomingNewHashesCount << " hashes";
                itemCount = c_maxIncomingNewHashesCount;
            }

            vector<pair<h256, u256>> hashes(itemCount);
            for (unsigned i = 0; i < itemCount; ++i)
                hashes[i] = std::make_pair(_r[i][0].toHash<h256>(), _r[i][1].toInt<u256>());

            m_peerObserver->onPeerNewHashes(_peerID, hashes);
            break;
        }
        case GetNodeDataPacket:
        {
            unsigned count = static_cast<unsigned>(_r.itemCount());
            if (!count)
            {
                LOG(m_loggerImpolite) << "Zero-entry GetNodeData: Not replying to " << _peerID;
                m_host->updateRating(_peerID, -10);
                break;
            }
            LOG(m_logger) << "GetNodeData (" << dec << count << " entries) from " << _peerID;

            strings const data = m_hostData->nodeData(_r);

            m_host->updateRating(_peerID, 0);
            RLPStream s;
            m_host->prep(_peerID, name(), s, NodeDataPacket, data.size());
            for (auto const& element : data)
                s.append(element);
            m_host->sealAndSend(_peerID, s);
            break;
        }
        case GetReceiptsPacket:
        {
            unsigned count = static_cast<unsigned>(_r.itemCount());
            if (!count)
            {
                LOG(m_loggerImpolite) << "Zero-entry GetReceipts: Not replying to " << _peerID;
                m_host->updateRating(_peerID, -10);
                break;
            }
            LOG(m_logger) << "GetReceipts (" << dec << count << " entries) from " << _peerID;

            pair<bytes, unsigned> const rlpAndItemCount = m_hostData->receipts(_r);

            m_host->updateRating(_peerID, 0);
            RLPStream s;
            m_host->prep(_peerID, name(), s, ReceiptsPacket, rlpAndItemCount.second)
                .appendRaw(rlpAndItemCount.first, rlpAndItemCount.second);
            m_host->sealAndSend(_peerID, s);
            break;
        }
        case NodeDataPacket:
        {
            if (remotePeer.asking() != Asking::NodeData)
                LOG(m_loggerImpolite)
                    << "Peer " << _peerID << " giving us node data when we didn't ask for them.";
            else
            {
                setIdle(_peerID);
                m_peerObserver->onPeerNodeData(_peerID, _r);
            }
            break;
        }
        case ReceiptsPacket:
        {
            if (remotePeer.asking() != Asking::Receipts)
                LOG(m_loggerImpolite)
                    << "Peer " << _peerID << " giving us receipts when we didn't ask for them.";
            else
            {
                setIdle(_peerID);
                m_peerObserver->onPeerReceipts(_peerID, _r);
            }
            break;
        }
        default:
            return false;
        }
    }
    catch (Exception const&)
    {
        LOG(m_loggerWarn) << "Peer " << _peerID << " causing an exception: "
                          << boost::current_exception_diagnostic_information() << " " << _r;
    }
    catch (std::exception const& _e)
    {
        LOG(m_loggerWarn) << "Peer " << _peerID << " causing an exception: " << _e.what() << " "
                          << _r;
    }

    return true;
}

void EthereumCapability::doBackgroundWork()
{
    ensureInitialised();
    auto h = m_chain.currentHash();
    // If we've finished our initial sync (including getting all the blocks into the chain so as to
    // reduce invalid transactions), start trading transactions & block hashes
    if (!isSyncing() && m_chain.isKnown(m_latestBlockHashSent))
    {
        if (m_newTransactions)
        {
            m_newTransactions = false;
            maintainTransactions();
        }
        if (m_newBlocks)
        {
            m_newBlocks = false;
            // Only propagate new block hashes as we send out new blocks immediately after
            // validating PoW
            maintainBlockHashes(h);
        }
    }

    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (now - m_lastTick >= 1)
    {
        m_lastTick = now;
        for (auto const& peer : m_peers)
        {
            time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            if (now - peer.second.lastAsk() > c_peerTimeoutSeconds && peer.second.isConversing())
                // timeout
                m_host->disconnect(peer.first, p2p::PingTimeout);
        }
    }
}

void EthereumCapability::setIdle(NodeID const& _peerID)
{
    setAsking(_peerID, Asking::Nothing);
}

void EthereumCapability::setAsking(NodeID const& _peerID, Asking _a)
{
    auto itPeerStatus = m_peers.find(_peerID);
    if (itPeerStatus == m_peers.end())
        return;

    auto& peerStatus = itPeerStatus->second;

    peerStatus.setAsking(_a);
    peerStatus.setLastAsk(std::chrono::system_clock::to_time_t(chrono::system_clock::now()));

    m_host->addNote(_peerID, "ask", ::toString(_a));
    m_host->addNote(_peerID, "sync",
        string(isCriticalSyncing(_peerID) ? "ONGOING" : "holding") +
            (needsSyncing(_peerID) ? " & needed" : ""));
}

bool EthereumCapability::isCriticalSyncing(NodeID const& _peerID) const
{
    auto itPeerStatus = m_peers.find(_peerID);
    if (itPeerStatus == m_peers.end())
        return false;

    auto const& peerStatus = itPeerStatus->second;

    auto const asking = peerStatus.asking();
    return asking == Asking::BlockHeaders || asking == Asking::State;
}

bool EthereumCapability::needsSyncing(NodeID const& _peerID) const
{
    if (m_host->isRude(_peerID, name()))
        return false;

    auto peerStatus = m_peers.find(_peerID);
    return (peerStatus != m_peers.end() && peerStatus->second.latestHash());
}

void EthereumCapability::disablePeer(NodeID const& _peerID, std::string const& _problem)
{
    m_host->disableCapability(_peerID, name(), _problem);
}

void EthereumCapability::removeSentTransactions(std::vector<h256> const& _txHashes)
{
    if (!_txHashes.empty())
    {
        // Function can be called from the client thread so we need to ensure that
        // m_transactionsSent modifications occur on the network thread
        m_host->postWork([_txHashes, this]() {
            for (auto const& txHash : _txHashes)
                m_transactionsSent.erase(txHash);
        });
    }
}

void EthereumCapability::propagateNewBlocks(std::shared_ptr<VerifiedBlocks const> const& _newBlocks)
{
    // Safe to call isSyncing() from a non-network thread since the underlying variable is
    // std::atomic
    if (_newBlocks->empty() || isSyncing())
        return;

    m_host->postWork([this, _newBlocks]() {
        // Verify that we're not too far behind - we perform this check on the network thread to
        // simplify the synchronization story (since otherwise we'd need to synchronize access to
        // m_latestBlockSent).
        auto const latestHash = _newBlocks->back().verified.info.hash();
        auto const detailsFrom = m_chain.details(m_latestBlockSent);
        auto const detailsTo = m_chain.details(latestHash);
        if (detailsFrom.totalDifficulty >= detailsTo.totalDifficulty)
            return;

        if (diff(detailsFrom.number, detailsTo.number) > c_maxSendNewBlocksCount)
        {
            m_latestBlockSent = latestHash;
            return;
        }

        auto const peersWithoutBlock = selectPeers(
            [&](EthereumPeer const& _peer) { return !_peer.isBlockKnown(latestHash); });

        auto const peersToSendNumber =
            std::max<std::size_t>(c_minBlockBroadcastPeers, std::sqrt(m_peers.size()));

        std::vector<NodeID> const peersToSend = randomPeers(peersWithoutBlock, peersToSendNumber);
        for (NodeID const& peerID : peersToSend)
            for (auto const& b : *_newBlocks)
            {
                RLPStream ts;
                m_host->prep(peerID, name(), ts, NewBlockPacket, 2)
                    .appendRaw(b.blockData, 1)
                    .append(b.verified.info.difficulty());

                auto itPeer = m_peers.find(peerID);
                if (itPeer != m_peers.end())
                {
                    m_host->sealAndSend(peerID, ts);
                    // We don't want to send new block hashes to these same peers
                    itPeer->second.markBlockAsKnown(b.verified.info.hash());
                }
            }
        if (!peersToSend.empty())
        {
            m_latestBlockSent = latestHash;
            LOG(m_logger) << "Sent " << _newBlocks->size() << " block(s) to " << peersToSend.size()
                          << " peers";
        }
    });
}

EthereumPeer const& EthereumCapability::peer(NodeID const& _peerID) const
{
    return const_cast<EthereumCapability*>(this)->peer(_peerID);
}

EthereumPeer& EthereumCapability::peer(NodeID const& _peerID)
{
    auto peer = m_peers.find(_peerID);
    if (peer == m_peers.end())
        BOOST_THROW_EXCEPTION(PeerDisconnected() << errinfo_nodeID(_peerID));

    return peer->second;
}
