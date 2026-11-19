// slang-ir-lower-bit-cast.h
#pragma once

// This file defines two IR passes for `BitCast`. `lowerOpaqueBitCast` runs before resource-type
// legalization and rewrites casts between structs that hold opaque handles field by field.
// `lowerBitCast` runs late and lowers a cast between aggregate types into bit-casts on
// basic-typed elements at matching byte offsets.

namespace Slang
{

struct IRModule;
class DiagnosticSink;
class TargetProgram;

void lowerBitCast(IRModule* module, TargetProgram* targetReq, DiagnosticSink* sink);

/// Rewrite each `BitCast` that has a struct holding an opaque handle (resource, sampler, buffer),
/// or an array of such structs, on either side. When the two types match position for position
/// (identical handle types, and opaque-free parts of equal natural size, alignment and offset),
/// the cast becomes a field-by-field rebuild with ordinary `BitCast`s on the opaque-free parts.
/// Other `BitCast`s, including those of a bare handle or an array of handles, are untouched.
///
/// We run this before resource-type legalization, which splits such structs apart and cannot
/// represent a `BitCast` between them, and on every target, so that a matched cast never needs
/// a byte representation of a handle.
///
/// When `diagnoseUnmatchedCasts` is set (targets that legalize resource types, where a handle
/// has no bytes), a cast that does not match is an error. Otherwise it is left for
/// `lowerBitCast`, which reinterprets the handles as bytes and reports an error when the
/// destination holds a handle.
void lowerOpaqueBitCast(
    IRModule* module,
    TargetProgram* targetProgram,
    bool diagnoseUnmatchedCasts,
    DiagnosticSink* sink);

} // namespace Slang
