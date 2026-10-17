// slang-ir-byte-address-legalize.h
#pragma once

namespace Slang
{
class Session;
class TargetProgram;
struct IRModule;
class DiagnosticSink;

struct ByteAddressBufferLegalizationOptions
{
    bool scalarizeVectorLoadStore = false;
    bool useBitCastFromUInt = false;
    bool translateToStructuredBufferOps = false;
    bool lowerBasicTypeOps = false;

    /// Causes all calls to `getEquivlentStructuredBuffer` to return a `ByteAddressBuffer` (this)
    /// instead of a `StructuredBuffer`. This option is used for targets that do not distinctly
    /// define `ByteAddressBuffer`/`StructuredBuffer` and introduce operations which prevent DCE
    /// from destroying old definitions of `ByteAddressBuffer` after variable replacement.
    bool treatGetEquivalentStructuredBufferAsGetThis = false;

    /// Report an error for stores of values with a scalar smaller than 4 bytes. Used for
    /// targets that can only write whole 32-bit words of a byte-address buffer, where such a
    /// store would have to read-modify-write the word and race with neighbouring writes.
    bool rejectSubWordStores = false;
};

/// Legalize byte-address buffer `Load()` and `Store()` operations.
///
/// This function translates load/store operations that involve
/// aggregate types into primitive load-store operations on
/// scalar or vector types.
///
void legalizeByteAddressBufferOps(
    IRModule* module,
    Session* session,
    TargetProgram* target,
    DiagnosticSink* sink,
    ByteAddressBufferLegalizationOptions const& options);
} // namespace Slang
