// slang-repro-validator.h
#ifndef SLANG_REPRO_VALIDATOR_H_INCLUDED
#define SLANG_REPRO_VALIDATOR_H_INCLUDED

#include "core/slang-list.h"

namespace Slang
{

/// Validate that a serialized repro buffer is safe to consume.
///
/// Verifies the buffer's structural integrity without dereferencing past it
/// and rejects buffers whose contents violate invariants the load path
/// assumes. Returns false on any violation; callers should emit
/// Diagnostics::InvalidReproState and abort the load.
bool isReproStateValid(const List<uint8_t>& buffer);

} // namespace Slang

#endif
