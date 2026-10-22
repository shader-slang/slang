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
    /// Split every constant-size vector and array load/store into per-element accesses, even when
    /// the access is known to be aligned to its whole size.
    bool scalarizeVectorLoadStore = false;
    bool useBitCastFromUInt = false;
    bool translateToStructuredBufferOps = false;
    bool lowerBasicTypeOps = false;

    /// Load `half` values by loading the containing 32-bit word and bit-casting it to
    /// `vector<half, 2>`, instead of going through a 16-bit integer. This is for targets
    /// that support `half` but have no 16-bit integer types (e.g. WGSL). It only affects loads.
    /// It requires `useBitCastFromUInt` and `lowerBasicTypeOps`, and not
    /// `translateToStructuredBufferOps`, because the word is read with a plain
    /// `ByteAddressBufferLoad` of a `uint`. `half` stores are unchanged: their read-modify-write
    /// of the containing word starts by bit-casting the `half` to a 16-bit integer.
    bool loadHalfFrom32BitWords = false;

    /// Causes all calls to `getEquivlentStructuredBuffer` to return a `ByteAddressBuffer` (this)
    /// instead of a `StructuredBuffer`. This option is used for targets that do not distinctly
    /// define `ByteAddressBuffer`/`StructuredBuffer` and introduce operations which prevent DCE
    /// from destroying old definitions of `ByteAddressBuffer` after variable replacement.
    bool treatGetEquivalentStructuredBufferAsGetThis = false;
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
