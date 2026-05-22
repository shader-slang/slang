// slang-repro-validator.h
#ifndef SLANG_REPRO_VALIDATOR_H_INCLUDED
#define SLANG_REPRO_VALIDATOR_H_INCLUDED

#include "core/slang-list.h"

namespace Slang
{

/// Validate the structural integrity of a serialized repro buffer.
///
/// Checks (without dereferencing past the buffer):
///   * the buffer is large enough to contain a RequestState at kStartOffset;
///   * every Offset32Ptr / Offset32Array / OffsetString reachable from
///     RequestState lies within the buffer (no out-of-bounds reads);
///   * OffsetString allocations are null-terminated and their size-encoding
///     header (1- to 4-byte multi-byte form) decodes to a length that fits;
///   * Offset32Array element counts do not overflow when multiplied by their
///     element size;
///   * FileState entries reachable from RequestState.files have a non-null
///     uniqueName whenever extractFiles() will dereference it (i.e. whenever
///     contents is set, or the FileState is reachable via pathInfoMap);
///   * SourceFileState.file is non-null and SourceFileState.file->contents is
///     non-null, so getSourceFile() can materialise a SourceFile from the blob;
///   * OutputState.entryPointIndex is within [0, entryPointCount) and
///     EntryPointState.translationUnitIndex is within [0, translationUnitCount).
///
/// Returns false on any violation; callers should emit
/// Diagnostics::InvalidReproState and abort the load.
bool isReproStateValid(const List<uint8_t>& buffer);

} // namespace Slang

#endif
