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

// For DXIL/HLSL targets at SM 6.7+, the DXR spec requires that "each field [of
// a `[raypayload]` struct] must declare one read and one write qualifier"
// (https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#payload-access-qualifiers).
// Slang's frontend only diagnoses fields where *both* qualifiers are missing,
// so two cases reach codegen with incomplete decorations: (1) structs tagged
// implicitly by `legalizeNonStructParameterToStructForHLSL`, and (2) explicit
// `[raypayload]` structs where the user wrote only `[read(...)]` or only
// `[write(...)]`. This pass walks each such struct and fills in whichever side
// is missing with the full `read/write(caller, anyhit, closesthit, miss)`
// default — preserving the implicit pre-SM-6.7 "any stage may access any field"
// semantics. Fields that already carry both qualifiers are left untouched.
void legalizeRayPayloadAccessQualifiersForHLSL(IRModule* module);

} // namespace Slang
