// slang-ir-hlsl-legalize.h
#pragma once
#include "core/slang-list.h"
#include "slang-compiler.h"
#include "slang-ir.h"

namespace Slang
{

class DiagnosticSink;
class Session;

struct IRModule;

void legalizeNonStructParameterToStructForHLSL(IRModule* module);

void legalizeEmptyRayPayloadsForHLSL(IRModule* module);

// Pad an empty callable-data struct with a dummy field so callable data survives type legalization
// on the D3D/HLSL path. An empty struct is otherwise erased during type legalization, which breaks
// two DXC requirements: a `[shader("callable")]` entry point loses its one required parameter, and
// a `CallShader(index, payload)` loses its payload argument. This pass finds the empty struct at
// both use sites — the callable entry point's `out`/`inout` parameter, and the second argument of
// a `CallShader` target-intrinsic call (recognized via `findTargetIntrinsicDefinition`, hence the
// `targetCaps`) — and pads it. The dummy field carries no payload access qualifiers (callable data
// is a plain `inout`, not a `[raypayload]` struct). See `legalizeEmptyCallableDataPayloadsForSPIRV`
// for the SPIR-V counterpart; a SPIR-V callable *entry-point parameter* needs no padding because it
// compiles to a valid `CallableKHR` entry point with no materialized variable.
void legalizeEmptyCallableDataPayloadsForHLSL(IRModule* module, CapabilitySet targetCaps);

// Pad an empty `CallShader` payload struct on the SPIR-V path. `CallShader`'s `spirv` arm backs the
// payload with a module-scope `[__vulkanCallablePayload]` global whose address feeds
// `OpExecuteCallableKHR`. An empty payload struct legalizes to `none`, leaving that instruction
// with a non-simple operand, and type legalization aborts with "non-simple operand(s)!". Keyed on
// the callable-payload global (not the call) so it does not depend on the intrinsic call surviving
// un-inlined; mirrors the global-var branch of `legalizeEmptyRayPayloadsForHLSL`. The dummy field
// carries no payload access qualifiers (callable data is a plain `inout`, not a `[raypayload]`).
void legalizeEmptyCallableDataPayloadsForSPIRV(IRModule* module);

// Fill in any missing per-side payload access qualifiers (PAQs) on every
// `[raypayload]` struct in the module, so that each field carries both a `read(...)`
// and a `write(...)` qualifier. HLSL SM 6.7+ requires both sides on every member of a
// `[raypayload]` struct; a user-authored struct with one-sided PAQ (or a struct that
// only reaches a hit shader and is never `TraceRay`'d) would otherwise be emitted with
// one-sided qualifiers and rejected by DXC.
void legalizeRayPayloadAccessQualifiersForHLSL(IRModule* module);

void validateBarrierFlagsForHLSL(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
