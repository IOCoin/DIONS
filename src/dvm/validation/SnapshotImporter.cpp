// Licensed under the GNU General Public License, Version 3.

#include "SnapshotImporter.h"
#include "Client.h"
#include "SnapshotStorage.h"

#include <libdevcore/FileSystem.h>
#include <libdevcore/RLP.h>
#include <libdevcore/TrieHash.h>
#include <libdvmashseal/Ethash.h>

#include <snappy.h>

namespace dev
{
namespace dvm
{

void SnapshotImporter::import(SnapshotStorageFace const& _snapshotStorage, h256 const& /*_genesisHash*/)
{
    bytes const manifestBytes = _snapshotStorage.readManifest();
    RLP manifest(manifestBytes);

    // For Snapshot format see https://github.com/paritytech/parity/wiki/Warp-Sync-Snapshot-Format
    u256 const version = manifest[0].toInt<u256>(RLP::VeryStrict);
    if (version != 2)
        BOOST_THROW_EXCEPTION(UnsupportedSnapshotManifestVersion());
    if (manifest.itemCount() != 6)
        BOOST_THROW_EXCEPTION(InvalidSnapshotManifest());

    u256 const blockNumber = manifest[4].toInt<u256>(RLP::VeryStrict);
    h256 const blockHash = manifest[5].toHash<h256>(RLP::VeryStrict);
    LOG(m_logger) << "Importing snapshot for block " << blockNumber << " block hash " << blockHash;

    h256s const stateChunkHashes = manifest[1].toVector<h256>(RLP::VeryStrict);
    h256 const stateRoot = manifest[3].toHash<h256>(RLP::VeryStrict);
    importStateChunks(_snapshotStorage, stateChunkHashes, stateRoot);

    h256s const blockChunkHashes = manifest[2].toVector<h256>(RLP::VeryStrict);
    importBlockChunks(_snapshotStorage, blockChunkHashes);
}

void SnapshotImporter::importStateChunks(SnapshotStorageFace const& _snapshotStorage, h256s const& _stateChunkHashes, h256 const& _stateRoot)
{
    size_t const stateChunkCount = _stateChunkHashes.size();

    size_t chunksImported = 0;
    size_t accountsImported = 0;

    for (auto const& stateChunkHash: _stateChunkHashes)
    {
        std::string const chunkUncompressed = _snapshotStorage.readChunk(stateChunkHash);

        RLP const accounts(chunkUncompressed);
        size_t const accountCount = accounts.itemCount();
        for (size_t accountIndex = 0; accountIndex < accountCount; ++accountIndex)
        {
            RLP const addressAndAccount = accounts[accountIndex];
            if (addressAndAccount.itemCount() != 2)
                BOOST_THROW_EXCEPTION(InvalidStateChunkData());

            h256 const addressHash = addressAndAccount[0].toHash<h256>(RLP::VeryStrict);
            if (!addressHash)
                BOOST_THROW_EXCEPTION(InvalidStateChunkData());

            // splitted parts of account can be only first in chunk
            if (accountIndex > 0 && m_stateImporter.isAccountImported(addressHash))
                BOOST_THROW_EXCEPTION(AccountAlreadyImported());

            RLP const account = addressAndAccount[1];
            if (account.itemCount() != 5)
                BOOST_THROW_EXCEPTION(InvalidStateChunkData());

            u256 const nonce = account[0].toInt<u256>(RLP::VeryStrict);
            u256 const balance = account[1].toInt<u256>(RLP::VeryStrict);

            RLP const storage = account[4];
            std::map<h256, bytes> storageMap;
            for (auto hashAndValue: storage)
            {
                if (hashAndValue.itemCount() != 2)
                    BOOST_THROW_EXCEPTION(InvalidStateChunkData());

                h256 const keyHash = hashAndValue[0].toHash<h256>(RLP::VeryStrict);
                if (!keyHash || storageMap.find(keyHash) != storageMap.end())
                    BOOST_THROW_EXCEPTION(InvalidStateChunkData());

                bytes value = hashAndValue[1].toBytes(RLP::VeryStrict);
                if (value.empty())
                    BOOST_THROW_EXCEPTION(InvalidStateChunkData());

                storageMap.emplace(keyHash, std::move(value));
            }

            byte const codeFlag = account[2].toInt<byte>(RLP::VeryStrict);
            h256 codeHash;
            switch (codeFlag)
            {
            case 0:
                codeHash = EmptySHA3;
                break;
            case 1:
                codeHash = m_stateImporter.importCode(account[3].toBytesConstRef(RLP::VeryStrict));
                break;
            case 2:
                codeHash = account[3].toHash<h256>(RLP::VeryStrict);
                if (!codeHash || m_stateImporter.lookupCode(codeHash).empty())
                    BOOST_THROW_EXCEPTION(InvalidStateChunkData());
                break;
            default:
                BOOST_THROW_EXCEPTION(InvalidStateChunkData());
            }

            m_stateImporter.importAccount(addressHash, nonce, balance, storageMap, codeHash);
        }
        accountsImported += accountCount;

        m_stateImporter.commitStateDatabase();

        ++chunksImported;
        LOG(m_logger) << "Imported chunk " << chunksImported << " (" << accounts.itemCount()
                      << " account records) Total account records imported: " << accountsImported;
        LOG(m_logger) << stateChunkCount - chunksImported << " chunks left to import";
    }

    // check root
    LOG(m_logger) << "Chunks imported: " << chunksImported;
    LOG(m_logger) << "Account records imported: " << accountsImported;
    LOG(m_logger) << "Reconstructed state root: " << m_stateImporter.stateRoot();
    LOG(m_logger) << "Manifest state root:      " << _stateRoot;
    if (m_stateImporter.stateRoot() != _stateRoot)
        BOOST_THROW_EXCEPTION(StateTrieReconstructionFailed());
}

void SnapshotImporter::importBlockChunks(SnapshotStorageFace const& _snapshotStorage, h256s const& _blockChunkHashes)
{
    size_t const blockChunkCount = _blockChunkHashes.size();
    size_t blockChunksImported = 0;
    // chunks are in decreasing order of first block number, so we go backwards to start from the oldest block
    for (auto chunk = _blockChunkHashes.rbegin(); chunk != _blockChunkHashes.rend(); ++chunk)
    {
        std::string const chunkUncompressed = _snapshotStorage.readChunk(*chunk);

        RLP blockChunk(chunkUncompressed);
        if (blockChunk.itemCount() < 3)
            BOOST_THROW_EXCEPTION(InvalidBlockChunkData());

        int64_t const firstBlockNumber = blockChunk[0].toPositiveInt64(RLP::VeryStrict);
        h256 const firstBlockHash = blockChunk[1].toHash<h256>(RLP::VeryStrict);
        u256 const firstBlockDifficulty = blockChunk[2].toInt<u256>(RLP::VeryStrict);
        if (!firstBlockNumber || !firstBlockHash || !firstBlockDifficulty)
            BOOST_THROW_EXCEPTION(InvalidBlockChunkData());

        LOG(m_logger) << "chunk first block " << firstBlockNumber << " first block hash "
                      << firstBlockHash << " difficulty " << firstBlockDifficulty;

        size_t const itemCount = blockChunk.itemCount();
        h256 parentHash = firstBlockHash;
        int64_t number = firstBlockNumber + 1;
        u256 totalDifficulty = firstBlockDifficulty;
        for (size_t i = 3; i < itemCount; ++i, ++number)
        {
            RLP blockAndReceipts = blockChunk[i];
            if (blockAndReceipts.itemCount() != 2)
                BOOST_THROW_EXCEPTION(InvalidBlockChunkData());

            RLP abridgedBlock = blockAndReceipts[0];

            BlockHeader header;
            header.setParentHash(parentHash);
            header.setAuthor(abridgedBlock[0].toHash<Address>(RLP::VeryStrict));

            h256 const blockStateRoot = abridgedBlock[1].toHash<h256>(RLP::VeryStrict);
            RLP transactions = abridgedBlock[8];
            h256 const txRoot = trieRootOver(transactions.itemCount(), [&](unsigned i) { return rlp(i); }, [&](unsigned i) { return transactions[i].data().toBytes(); });
            RLP uncles = abridgedBlock[9];
            RLP receipts = blockAndReceipts[1];
            std::vector<bytesConstRef> receiptsVector;
            for (auto receipt: receipts)
                receiptsVector.push_back(receipt.data());
            h256 const receiptsRoot = orderedTrieRoot(receiptsVector);
            h256 const unclesHash = sha3(uncles.data());
            header.setRoots(txRoot, receiptsRoot, unclesHash, blockStateRoot);

            header.setLogBloom(abridgedBlock[2].toHash<LogBloom>(RLP::VeryStrict));
            u256 const difficulty = abridgedBlock[3].toInt<u256>(RLP::VeryStrict);
            header.setDifficulty(difficulty);
            header.setNumber(number);
            header.setGasLimit(abridgedBlock[4].toInt<u256>(RLP::VeryStrict));
            header.setGasUsed(abridgedBlock[5].toInt<u256>(RLP::VeryStrict));
            header.setTimestamp(abridgedBlock[6].toPositiveInt64(RLP::VeryStrict));
            header.setExtraData(abridgedBlock[7].toBytes(RLP::VeryStrict));

            Ethash::setMixHash(header, abridgedBlock[10].toHash<h256>(RLP::VeryStrict));
            Ethash::setNonce(header, abridgedBlock[11].toHash<Nonce>(RLP::VeryStrict));

            totalDifficulty += difficulty;
            m_blockChainImporter.importBlock(header, transactions, uncles, receipts, totalDifficulty);

            parentHash = header.hash();
        }

        LOG(m_logger) << "Imported chunk " << *chunk << " (" << itemCount - 3 << " blocks)";
        LOG(m_logger) << blockChunkCount - (++blockChunksImported) << " chunks left to import";

        if (chunk == _blockChunkHashes.rbegin())
        {
            LOG(m_logger) << "Setting chain start block: " << firstBlockNumber + 1;
            m_blockChainImporter.setChainStartBlockNumber(firstBlockNumber + 1);
        }
    }
}

}  // namespace dvm
}  // namespace dev
