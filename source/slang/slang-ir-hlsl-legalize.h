// slang-ir-hlsl-legalize.h
#pragma once
#include "../core/slang-list.h"
#include "slang-compiler.h"
#include "slang-ir.h"

namespace Slang
{

class DiagnosticSink;
class Session;

struct IRFunc;
struct IRModule;

bool isBarrierFlagGetterOp(IROp op);
IRInst* getBarrierFlagValueInst(IRInst* inst);
uint32_t getKnownBarrierMemoryTypeFlags();
uint32_t getKnownBarrierSemanticFlags();
bool isValidBarrierMemoryTypeFlags(uint32_t flagVal);
bool isValidBarrierSemanticFlags(uint32_t flagVal);
bool isBarrierFlagValueCast(IRInst* castInst, IRType* fromType, IRType* toType);

void legalizeNonStructParameterToStructForHLSL(IRModule* module);

void legalizeEmptyRayPayloadsForHLSL(IRModule* module);

// Fill in any missing per-side payload access qualifiers (PAQs) on every
// `[raypayload]` struct in the module, so that each field carries both a `read(...)`
// and a `write(...)` qualifier. HLSL SM 6.7+ requires both sides on every member of a
// `[raypayload]` struct; a user-authored struct with one-sided PAQ (or a struct that
// only reaches a hit shader and is never `TraceRay`'d) would otherwise be emitted with
// one-sided qualifiers and rejected by DXC.
void legalizeRayPayloadAccessQualifiersForHLSL(IRModule* module);

void validateBarrierFlagsForHLSL(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
