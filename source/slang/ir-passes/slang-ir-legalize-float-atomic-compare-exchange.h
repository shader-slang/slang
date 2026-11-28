// slang-ir-legalize-float-atomic-compare-exchange.h
#pragma once

namespace Slang
{
struct IRModule;
class TargetRequest;
class DiagnosticSink;
enum class CodeGenTarget;

/// Rewrite each floating-point `AtomicCompareExchange` as an integer one, for SPIR-V and GLSL.
///
/// SPIR-V's `OpAtomicCompareExchange` and GLSL's `atomicCompSwap` take integer operands only, so
/// a compare-exchange on a `float` or `double` is rewritten as one on the unsigned integer of the
/// same width. The comparison becomes a bitwise one, as with HLSL's
/// `InterlockedCompareExchangeFloatBitwise`.
///
/// A logical SPIR-V pointer cannot be bitcast, so the integer operation goes through a second
/// declaration of the buffer, with integer elements and the same binding. That works when the
/// destination is in a `RWStructuredBuffer` global shader parameter, reached through fields and
/// array elements whose offsets are known. A destination in a physical storage buffer is reached
/// by bitcasting its pointer when `target` is SPIR-V. GLSL has no such bitcast, so that
/// includes SPIR-V emitted through GLSL, where `target` is GLSL. Any other destination, and a
/// `half` value, is diagnosed.
///
void legalizeFloatAtomicCompareExchange(
    IRModule* module,
    TargetRequest* targetReq,
    CodeGenTarget target,
    DiagnosticSink* sink);

} // namespace Slang
