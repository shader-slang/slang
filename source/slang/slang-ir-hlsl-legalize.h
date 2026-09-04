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

// Pad an empty callable-data struct with a dummy field so a `[shader("callable")]` entry point
// keeps a legal one-parameter ABI on the D3D/HLSL path. An empty struct is otherwise erased
// during type legalization, leaving a zero-parameter callable that DXC rejects. The dummy field
// carries no payload access qualifiers (callable data is a plain `inout` parameter, not a
// `[raypayload]` struct). D3D-only, and scoped to the callable entry-point parameter: on SPIR-V an
// empty callable-data *entry-point parameter* compiles to a valid `CallableKHR` entry point with no
// materialized variable, so no padding is needed there. (A `CallShader` *call site* with an empty
// payload is a separate empty-type-on-SPIR-V concern this pass does not address.)
void legalizeEmptyCallableDataPayloadsForHLSL(IRModule* module);

// Fill in any missing per-side payload access qualifiers (PAQs) on every
// `[raypayload]` struct in the module, so that each field carries both a `read(...)`
// and a `write(...)` qualifier. HLSL SM 6.7+ requires both sides on every member of a
// `[raypayload]` struct; a user-authored struct with one-sided PAQ (or a struct that
// only reaches a hit shader and is never `TraceRay`'d) would otherwise be emitted with
// one-sided qualifiers and rejected by DXC.
void legalizeRayPayloadAccessQualifiersForHLSL(IRModule* module);

void validateBarrierFlagsForHLSL(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
