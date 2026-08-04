// slang-repro-validator.h
#ifndef SLANG_REPRO_VALIDATOR_H_INCLUDED
#define SLANG_REPRO_VALIDATOR_H_INCLUDED

#include "core/slang-list.h"

namespace Slang
{

/// Validate that a serialized repro buffer is safe to consume.
///
/// The buffer must be the post-RIFF-unwrapped repro state payload, with
/// RequestState placed at kStartOffset.
///
/// Checks offset and array bounds, alignment, OffsetString encodings and
/// terminators, required file/string relationships, and entry point/output
/// index ranges. Returns false on any violation; callers should emit
/// Diagnostics::InvalidReproState, discard the invalid buffer, and abort the
/// load. ReproUtil::loadState implements that rejection contract.
[[nodiscard]] bool isReproStateValid(const List<uint8_t>& buffer);

} // namespace Slang

#endif
