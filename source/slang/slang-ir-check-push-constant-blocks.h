#pragma once

#include "core/slang-basic.h"

namespace Slang
{
struct IRFunc;
struct IRInst;
struct IRModule;
class DiagnosticSink;

/// Report an error for every entry point that statically uses more than one push-constant block.
///
/// Vulkan allows at most one push-constant block to be statically used per entry point. Consider a
/// global `[[vk::push_constant]] uniform PC push;` together with a compute entry point
/// `void main(uniform float k)`: on Khronos targets the `uniform` parameters of compute and
/// rasterization entry points are placed in a second push-constant block, so an entry point that
/// reads both `push` and `k` would produce SPIR-V that fails
/// `VUID-StandaloneSpirv-OpEntryPoint-06674` and GLSL that glslang rejects.
///
/// `referencingEntryPoints` maps each global instruction to the entry points whose call trees use
/// it, which is the static use that the Vulkan rule counts. Each target calls this where that
/// graph describes the code it emits: SPIR-V after the last dead-code elimination in
/// `legalizeIRForSPIRV`, and GLSL when the emitter starts.
/// `isPushConstantBlock` is a parameter because the targets recognize push-constant blocks
/// differently at that point: the GLSL emitter still has each parameter's var layout, while SPIR-V
/// legalization has replaced the layout of a push-constant block with a `PushConstant` pointer
/// type.
void checkForMultiplePushConstantBlocksPerEntryPoint(
    IRModule* module,
    Dictionary<IRInst*, HashSet<IRFunc*>>& referencingEntryPoints,
    bool (*isPushConstantBlock)(IRInst* globalParam),
    DiagnosticSink* sink);

/// Return true if `globalParam` is bound as a push-constant block by its var layout. The GLSL
/// emitter declares such a parameter with `layout(push_constant)`, and SPIR-V legalization gives it
/// the `PushConstant` address space, because both test the container var layout for a
/// `PushConstantBuffer` resource.
bool isPushConstantBlockByLayout(IRInst* globalParam);

} // namespace Slang
