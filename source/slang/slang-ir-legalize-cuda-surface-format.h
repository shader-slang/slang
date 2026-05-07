// slang-ir-legalize-cuda-surface-format.h
#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;

/// Legalize CUDA surface format conversions.
///
/// For CUDA targets, surface read/write operations are raw byte operations
/// with no hardware format conversion. This pass rewrites surface calls that
/// have a format decoration (indicating a mismatch between storage format and
/// access type) to:
///   1. Change the texture element type to the raw storage type
///   2. Insert inline conversion arithmetic (decode on read, encode on write)
///   3. Remove the format decoration so the emitter sees plain typed access
///
void legalizeCUDASurfaceFormat(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
