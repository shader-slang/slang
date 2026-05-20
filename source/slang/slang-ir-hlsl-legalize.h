// slang-ir-hlsl-legalize.h
#pragma once
#include "../core/slang-list.h"
#include "slang-compiler.h"

namespace Slang
{

class DiagnosticSink;
class Session;

struct IRFunc;
struct IRModule;

void legalizeNonStructParameterToStructForHLSL(IRModule* module);

void legalizeEmptyRayPayloadsForHLSL(IRModule* module);

// For DXIL/HLSL targets at SM 6.7+, DXC requires every field of a `[raypayload]`
// struct to carry payload access qualifiers. When Slang implicitly tags a struct
// as a ray payload (e.g. via `__forceVarIntoRayPayloadStructTemporarily`) the
// fields lack qualifiers. This pass attaches default `read/write(caller, anyhit,
// closesthit, miss)` decorations to those fields, matching pre-SM 6.7 implicit
// "any stage may access any field" semantics. Fields that already carry either a
// read or a write decoration are left untouched.
void legalizeRayPayloadAccessQualifiersForHLSL(IRModule* module);

} // namespace Slang
