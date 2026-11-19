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
/// The CUDA emitter and `checkEntryPointNamesAreUnique` both build the kernel symbol from it.
UnownedStringSlice getOptiXEntryPointNamePrefix(Stage stage);

/// Return whether the C-like emitter for the code-generation format `target` replaces the
/// entry-point name `entryPointName` with a generated unique name (warning E40100). The CPU, CUDA
/// and Metal source emitters do so for `main`, which cannot name a kernel in the C++-based
/// languages that they write. `target` is the format the emitter writes
/// (`CodeGenContext::getTargetFormat()`), such as CUDA source for a PTX target: a final format
/// such as PTX is not a source format, so the answer for it is false. The C-like emitter and
/// `checkEntryPointNamesAreUnique` both ask with that format, so they agree on which names change.
bool doesTargetRenameEntryPointName(CodeGenTarget target, UnownedStringSlice entryPointName);

/// Report E38016 for each entry point in `entryPoints` that the emitter for `codeGenContext`
/// would write under the same symbol as an earlier entry point of the list, with a note at that
/// earlier one. Consider this example:
///
///     [shader("compute")] [numthreads(1, 1, 1)] void Count(uniform uint a) { ... }
///     [shader("compute")] [numthreads(1, 1, 1)] void Count(uniform float a) { ... }
///
/// The emitters write each entry point under its unqualified name, so CUDA and Metal would emit
/// two kernels named `Count` and SPIR-V two `OpEntryPoint GLCompute ... "Count"`, which the
/// downstream compiler or validator rejects.
///
/// `entryPoints` must be the entry points of the program being compiled (`LinkedIR::entryPoints`),
/// not every function with an `IREntryPointDecoration`: a `[shader]` function that the program
/// only calls keeps its decoration after linking but is not an entry point of the output. The same
/// function listed twice is not a collision. Entry points that the emitter does not write under
/// their decoration name are not compared (see `computeEntryPointSymbolKey`): every entry point on
/// GLSL and the Slang VM, `main` on the CPU, CUDA and Metal source formats, and every non-compute
/// entry point on the CPU formats that the LLVM emitter writes. Collisions can only
/// occur when several entry points share one output, as in a whole-program compile.
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
