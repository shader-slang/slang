// slang-ir-entry-point-decorations.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
enum class CodeGenTarget;
struct CodeGenContext;
class DiagnosticSink;

/// Checks entry point decoration values to ensure that they are valid for
/// the shader stage and target.
void checkEntryPointDecorations(IRModule* module, CodeGenTarget target, DiagnosticSink* sink);

/// Return the prefix that OptiX requires on the global symbol of a CUDA entry point for `stage`,
/// for example `__raygen__` for ray generation. OptiX identifies the program type of a kernel by
/// this prefix. Stages that are not OptiX program types, such as compute, have an empty prefix.
UnownedStringSlice getOptiXEntryPointNamePrefix(Stage stage);

/// Return whether the emitter for the code-generation format `target` replaces the entry-point
/// name `entryPointName` with a generated unique name (warning E40100). The C-like source
/// emitters for CPU, CUDA and Metal do so for `main`, which cannot name a kernel in the C++-based
/// languages that they write.
bool doesTargetRenameEntryPointName(CodeGenTarget target, UnownedStringSlice entryPointName);

/// Diagnose two entry points of the linked program `entryPoints` that the emitter for the target
/// of `codeGenContext` would write under the same symbol, such as two overloads of one
/// `[shader("compute")]` function. Consider this example:
///
///     [shader("compute")] [numthreads(1, 1, 1)] void Count(uniform uint a) { ... }
///     [shader("compute")] [numthreads(1, 1, 1)] void Count(uniform float a) { ... }
///
/// The emitters write each entry point under its unqualified name, so CUDA and Metal would emit
/// two kernels named `Count` and SPIR-V two `OpEntryPoint GLCompute ... "Count"`, which the
/// downstream compiler or validator rejects. SPIR-V allows entry points of different execution
/// models to share a name, and CUDA prefixes the symbol of an OptiX ray-tracing entry point with
/// its program type, so the comparison uses the symbol that each emitter writes.
///
/// `entryPoints` holds only the entry points of the program being compiled. A `[shader]` function
/// that the program calls as an ordinary function is not an entry point of the output.
void checkEntryPointNamesAreUnique(
    CodeGenContext* codeGenContext,
    const List<IRFunc*>& entryPoints);


// OutputTopologyType member definition macro
#define OUTPUT_TOPOLOGY_TYPES(M) \
    M(Point, point)              \
    M(Line, line)                \
    M(Triangle, triangle)        \
    M(TriangleCW, triangle_cw)   \
    M(TriangleCCW, triangle_ccw) \
    /* end */

enum class OutputTopologyType
{
    Unknown = 0,
#define CASE(ID, NAME) ID,
    OUTPUT_TOPOLOGY_TYPES(CASE)
#undef CASE
};


OutputTopologyType convertOutputTopologyStringToEnum(String rawOutputTopology);

} // namespace Slang
