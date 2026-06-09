#pragma once

namespace Slang
{
struct IRInst;
struct IRModule;

enum class NonUniformResourceIndexFloatMode
{
    Textual,
    SPIRV,
};

void processNonUniformResourceIndex(
    IRInst* nonUniformResourceIndexInst,
    NonUniformResourceIndexFloatMode floatMode);

void floatNonUniformResourceIndex(IRModule* module, NonUniformResourceIndexFloatMode floatMode);

// Phase 2 of the SPIR-V NonUniform path: a single forward scan that decorates
// the post-legalization, emit-shape resource operands. Must be invoked as the
// FINAL step of SPIR-V legalization (see the file header in the .cpp for the
// ordering rationale). SPIR-V only; the textual path does not call this.
void propagateNonUniformDecorations(IRModule* module);

} // namespace Slang
