// slang-ir-lower-bit-cast.h
#pragma once

// This file defines an IR pass that lowers a BitCast<T>(U) operation, where T and U are struct
// types, into a series of bit-cast operations on basic-typed elements.

namespace Slang
{

struct IRModule;
class DiagnosticSink;
class TargetProgram;

void lowerBitCast(IRModule* module, TargetProgram* targetReq, DiagnosticSink* sink);

/// Rewrite each `BitCast` between aggregates that contain opaque handles (resources, samplers,
/// buffers) into a field-by-field rebuild when the two types match position-for-position:
/// opaque handles of identical type, and opaque-free parts of equal natural size and alignment.
/// This runs before resource-type legalization, which splits such aggregates apart and has no
/// representation for a `BitCast` between them.
///
/// When `diagnoseUnmatchedCasts` is set, a cast that does not match is reported as an error,
/// because the target gives opaque handles no byte representation. Otherwise the cast is left
/// for `lowerBitCast`, which treats the handles as ordinary bytes.
void lowerOpaqueBitCast(
    IRModule* module,
    TargetProgram* targetProgram,
    bool diagnoseUnmatchedCasts,
    DiagnosticSink* sink);

} // namespace Slang
