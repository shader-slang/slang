#ifndef SLANG_CHECK_SYNTHESIZE_COVERAGE_H
#define SLANG_CHECK_SYNTHESIZE_COVERAGE_H

namespace Slang
{
class ModuleDecl;
struct SemanticsVisitor;

// When `-trace-coverage` is enabled, synthesize a module-scope
// `RWStructuredBuffer<uint> __slang_coverage` declaration in the
// given ModuleDecl, unless the user has already declared one.
//
// Runs at semantic-check time, before parameter binding. The
// synthesized decl goes through the normal binding/reflection/IR
// pipeline, so every backend and every reflection-driven runtime
// (slang-rhi, slangpy, user Vulkan/D3D12 apps) sees the buffer as
// a first-class shader parameter.
//
// No-op when the flag is off or on core-module modules.
void maybeSynthesizeCoverageBufferDecl(SemanticsVisitor* visitor, ModuleDecl* moduleDecl);

} // namespace Slang

#endif // SLANG_CHECK_SYNTHESIZE_COVERAGE_H
