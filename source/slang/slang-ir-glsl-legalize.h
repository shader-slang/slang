// slang-ir-glsl-legalize.h
#pragma once
#include "core/slang-list.h"
#include "slang-compiler.h"

namespace Slang
{

class DiagnosticSink;
class Session;

class ShaderExtensionTracker;

struct IRFunc;
struct IRModule;

void legalizeEntryPointsForGLSL(
    IRModule* module,
    Session* session,
    const List<IRFunc*>& func,
    CodeGenContext* context,
    ShaderExtensionTracker* glslExtensionTracker);

// GLSL, SPIR-V, and WGSL all require an integer `switch` selector, but the front end accepts
// a `switch` on a `bool` (either `case true:`/`case false:`, or a switch on an `enum : bool`)
// and lowers it unchanged. Rewrite every such switch in `module` into the equivalent integer
// switch so these emitters never see a boolean selector. Run for the targets that need an
// integer selector; the C-family and Metal emitters accept a bool switch directly.
void legalizeBoolSwitchForTargetsRequiringIntSwitch(IRModule* module);

// Rewrite the `FRem` shapes the GLSL text emitter cannot spell as valid GLSL:
// every matrix-typed `FRem` becomes one `FRem` per row (scalar operands
// splatted to the row vector type), and a vector-typed `FRem` with a scalar
// dividend has the dividend splatted to the vector type — GLSL `mod` has no
// matrix overloads and no `mod(float, genType)` overload. Run for GLSL and
// SPIR-V-via-GLSL only; the direct SPIR-V emitter handles both shapes itself
// and keeps the original instructions. The full rationale lives with the
// definitions in slang-ir-glsl-legalize.cpp.
void legalizeFRemForGLSL(IRModule* module);

void legalizeConstantBufferLoadForGLSL(IRModule* module);

void legalizeDispatchMeshPayloadForGLSL(IRModule* module);

void legalizeDynamicResourcesForGLSL(IRModule* module, CodeGenContext* context);
} // namespace Slang
