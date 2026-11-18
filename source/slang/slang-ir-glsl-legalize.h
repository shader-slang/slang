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

// GLSL provides no matrix overloads for `sign`, `abs`, or `mod`, so a
// matrix-typed floating-point remainder (`%`, a matrix `IRFRem`) cannot be
// emitted as text: the GLSL construction `sign(x) * mod(abs(x), abs(y))` only
// type-checks for scalar and vector operands. Rewrite every matrix-typed
// `FRem` in `module` into one `FRem` per row (scalar operands splatted to the
// row vector type) and reassemble the matrix from the row results, so the
// GLSL-family emitters only ever see vector-shaped `FRem`s. Run for GLSL and
// SPIR-V-via-GLSL only; the direct SPIR-V emitter already expands matrix
// arithmetic per row at emit time, so it keeps the matrix-typed `FRem`.
void legalizeMatrixFRemForGLSL(IRModule* module);

void legalizeConstantBufferLoadForGLSL(IRModule* module);

void legalizeDispatchMeshPayloadForGLSL(IRModule* module);

void legalizeDynamicResourcesForGLSL(IRModule* module, CodeGenContext* context);
} // namespace Slang
