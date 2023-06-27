// Copyright 2014-2019 Aleth Authors.
// Copyright 2022-2023 blastdoor7.
// Licensed under the GNU General Public License, Version 3.
#pragma once

#include "ptrie/vector_ref.h"

namespace dev
{
namespace db
{
using Slice = vector_ref<char const>;
class WriteBatchFace;
class DatabaseFace;
}
}
