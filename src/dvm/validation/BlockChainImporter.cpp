// Licensed under the GNU General Public License, Version 3.

#include "BlockChainImporter.h"
#include "BlockChain.h"

#include <libdevcore/RLP.h>
#include <libdvmcore/BlockHeader.h>

namespace dev
{
namespace dvm
{

namespace
{

class BlockChainImporter: public BlockChainImporterFace
{
public:
	explicit BlockChainImporter(BlockChain& _blockChain): m_blockChain(_blockChain) {}

	void importBlock(BlockHeader const& _header, RLP _transactions, RLP _uncles, RLP _receipts, u256 const& _totalDifficulty) override
	{
		RLPStream headerRlp;
		_header.streamRLP(headerRlp);

		RLPStream block(3);
		block.appendRaw(headerRlp.out());
		block << _transactions << _uncles;

		m_blockChain.insertWithoutParent(block.out(), _receipts.data(), _totalDifficulty);
	}

	void setChainStartBlockNumber(u256 const& _number) override
	{
		m_blockChain.setChainStartBlockNumber(static_cast<unsigned>(_number));
	}

private:
	BlockChain& m_blockChain;
};

}

std::unique_ptr<BlockChainImporterFace> createBlockChainImporter(BlockChain& _blockChain)
{
	return std::unique_ptr<BlockChainImporterFace>(new BlockChainImporter(_blockChain));
}

}
}
