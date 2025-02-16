// Licensed under the GNU General Public License, Version 3.

#pragma once

#include <mutex>
#include <unordered_map>

#include <libdevcore/Guards.h>
#include <libdvmcore/Common.h>
#include <libdvmcore/BlockHeader.h>
#include <libp2p/Common.h>
#include "CommonNet.h"

namespace dev
{

class RLPStream;

namespace dvm
{
class dvmCapability;
class BlockQueue;
class dvmPeer;

/**
 * @brief Base BlockChain synchronization strategy class.
 * Syncs to peers and keeps up to date. Base class handles blocks downloading but does not contain any details on state transfer logic.
 */
class BlockChainSync final: public HasInvariants
{
public:
    explicit BlockChainSync(dvmCapability& _host);
    ~BlockChainSync();
    void abortSync(); ///< Abort all sync activity

    /// @returns true is Sync is in progress
    bool isSyncing() const;

    /// Restart sync
    void restartSync();

    /// Called after all blocks have been downloaded
    /// Public only for test mode
    void completeSync();

    /// Called by peer to report status
    void onPeerStatus(dvmPeer const& _peer);

    /// Called by peer once it has new block headers during sync
    void onPeerBlockHeaders(NodeID const& _peerID, RLP const& _r);

    /// Called by peer once it has new block bodies
    void onPeerBlockBodies(NodeID const& _peerID, RLP const& _r);

    /// Called by peer once it has new block bodies
    void onPeerNewBlock(NodeID const& _peerID, RLP const& _r);

    void onPeerNewHashes(NodeID const& _peerID, std::vector<std::pair<h256, u256>> const& _hashes);

    /// Called by peer when it is disconnecting
    void onPeerAborting();

    /// Called when a blockchain has imported a new block onto the DB
    void onBlockImported(BlockHeader const& _info);

    /// @returns Synchonization status
    SyncStatus status() const;

    static char const* stateName(SyncState _s) { return s_stateNames[static_cast<int>(_s)]; }

private:
    /// Resume downloading after witing state
    void continueSync();

    /// Enter waiting state
    void pauseSync() { m_state = SyncState::Waiting; }
    bool isSyncPaused() { return m_state == SyncState::Waiting; }

    dvmCapability& host() { return m_host; }
    dvmCapability const& host() const { return m_host; }

    void resetSync();
    void syncPeer(NodeID const& _peerID, bool _force);
    void requestBlocks(NodeID const& _peerID);
    void clearPeerDownload(NodeID const& _peerID);
    void clearPeerDownload();
    void collectBlocks();
    bool requestDaoForkBlockHeader(NodeID const& _peerID);
    bool verifyDaoChallengeResponse(RLP const& _r);
    void logImported(unsigned _success, unsigned _future, unsigned _got, unsigned _unknown);

private:
    struct Header
    {
        bytes data;		///< Header data
        h256 hash;		///< Block hash
        h256 parent;	///< Parent hash
    };

    /// Used to identify header by transactions and uncles hashes
    struct HeaderId
    {
        h256 transactionsRoot;
        h256 uncles;

        bool operator==(HeaderId const& _other) const
        {
            return transactionsRoot == _other.transactionsRoot && uncles == _other.uncles;
        }
    };

    struct HeaderIdHash
    {
        std::size_t operator()(const HeaderId& _k) const
        {
            size_t seed = 0;
            h256::hash hasher;
            boost::hash_combine(seed, hasher(_k.transactionsRoot));
            boost::hash_combine(seed, hasher(_k.uncles));
            return seed;
        }
    };

    dvmCapability& m_host;

    // Triggered once blocks have been drained from the block queue,  potentially freeing up space
    // for more blocks. Note that the block queue can still be full after a drain, depending on how
    // many blocks are in the queue vs how many are being drained.
    Handler<> m_bqBlocksDrained;

    mutable RecursiveMutex x_sync;
    /// Peers to which we've sent DAO request
    std::set<NodeID> m_daoChallengedPeers;
    std::atomic<SyncState> m_state{SyncState::Idle};		///< Current sync state
    h256Hash m_knownNewHashes; 					///< New hashes we know about use for logging only
    unsigned m_chainStartBlock = 0;
    unsigned m_startingBlock = 0;      	    	///< Last block number for the start of sync
    unsigned m_highestBlock = 0;       	     	///< Highest block number seen
    std::unordered_set<unsigned> m_downloadingHeaders;		///< Set of block body numbers being downloaded
    std::unordered_set<unsigned> m_downloadingBodies;		///< Set of block header numbers being downloaded
    std::map<unsigned, std::vector<Header>> m_headers;	    ///< Downloaded headers
    std::map<unsigned, std::vector<bytes>> m_bodies;	    ///< Downloaded block bodies
    /// Peers to m_downloadingHeaders number map
    std::map<NodeID, std::vector<unsigned>> m_headerSyncPeers;
    /// Peers to m_downloadingBodies number map
    std::map<NodeID, std::vector<unsigned>> m_bodySyncPeers;
    std::unordered_map<HeaderId, unsigned, HeaderIdHash> m_headerIdToNumber;
    bool m_haveCommonHeader = false;			///< True if common block for our and remote chain has been found
    unsigned m_lastImportedBlock = 0; 			///< Last imported block number
    h256 m_lastImportedBlockHash;				///< Last imported block hash
    u256 m_syncingTotalDifficulty;				///< Highest peer difficulty

    Logger m_logger{createLogger(VerbosityDebug, "sync")};
    Logger m_loggerInfo{createLogger(VerbosityInfo, "sync")};
    Logger m_loggerDetail{createLogger(VerbosityTrace, "sync")};
    Logger m_loggerWarning{createLogger(VerbosityWarning, "sync")};

private:
    static char const* const s_stateNames[static_cast<int>(SyncState::Size)];
    bool invariants() const override;
    void logNewBlock(h256 const& _h);
};

std::ostream& operator<<(std::ostream& _out, SyncStatus const& _sync);

}
}
