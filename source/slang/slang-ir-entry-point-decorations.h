// slang-ir-entry-point-decorations.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
enum class CodeGenTarget;
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
