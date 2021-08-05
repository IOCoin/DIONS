// Licensed under the GNU General Public License, Version 3.

#include "DVMSchedule.h"
#include <libdvmcore/ChainOperationParams.h>

namespace dev
{
namespace dvm
{
DVMSchedule::DVMSchedule(DVMSchedule const& _schedule, AdditionalEIPs const& _eips)
  : DVMSchedule(_schedule)
{
    if (_eips.eip1380)
        callSelfGas = 40;
    if (_eips.eip2046)
        precompileStaticCallGas = 40;
}
}  // namespace dvm
}  // namespace dev
