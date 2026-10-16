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
/// Vulkan allows at most one push-constant block per entry point, and Slang lays out every block
/// from offset 0. Consider a global `[[vk::push_constant]] uniform PC push;` together with an
/// entry point `void main(uniform float k)`: on Khronos targets `k` is placed in a second
/// push-constant block, so an entry point that reads both `push` and `k` would produce SPIR-V
/// that fails `VUID-StandaloneSpirv-OpEntryPoint-06674` and GLSL that glslang rejects.
///
/// `isPushConstantBlock` decides which global parameters are push-constant blocks, and
/// `referencingEntryPoints` maps each global instruction to the entry points that reach it. Each
/// target calls this with the same classification and reference graph that its emitter later uses
/// to declare the entry point's interface, so the check reports the modules the emitter would make
/// invalid.
void checkForMultiplePushConstantBlocksPerEntryPoint(
    IRModule* module,
    Dictionary<IRInst*, HashSet<IRFunc*>>& referencingEntryPoints,
    bool (*isPushConstantBlock)(IRInst* globalParam),
    DiagnosticSink* sink);

/// Return true if `globalParam` is bound as a push-constant block by its var layout, which is how
/// the GLSL emitter decides to declare it with `layout(push_constant)`.
bool isPushConstantBlockByLayout(IRInst* globalParam);

} // namespace Slang
