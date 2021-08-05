// Licensed under the GNU General Public License, Version 3.


#pragma once

#include <libdevcrypto/Common.h>
#include "ValidationEngine.h"

namespace dev
{
namespace dvm
{

class BasicAuthority: public ValidationEngineBase
{
public:
    static std::string name() { return "BasicAuthority"; }
    unsigned revision() const override { return 0; }
	unsigned validationFields() const override { return 1; }
	bytes validationRLP() const override { return rlp(Signature()); }

	void populateFromParent(BlockHeader&, BlockHeader const&) const override;
	StringHashMap jsInfo(BlockHeader const& _bi) const override;
	void verify(Strictness _s, BlockHeader const& _bi, BlockHeader const& _parent, bytesConstRef _block) const override;
	bool shouldValidation(Interface*) override;
	void generateValidation(BlockHeader const& _bi) override;

	static Signature sig(BlockHeader const& _bi) { return _bi.validation<Signature>(); }
	static BlockHeader& setSig(BlockHeader& _bi, Signature const& _sig) { _bi.setValidation(_sig); return _bi; }
	void setSecret(Secret const& _s) { m_secret = _s; }
	static void init();

private:
	bool onOptionChanging(std::string const& _name, bytes const& _value) override;

	Secret m_secret;
	AddressHash m_authorities;
};

}
}
