#pragma once

namespace Slang
{

struct IRModule;
class TargetProgram;
class DiagnosticSink;

/// Replace `GetDynamicResourceHeap` insts with an actual array of resources.
void lowerDynamicResourceHeap(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

/// Lower the untyped descriptor-heap handle types (`UntypedResourceHandleType` /
/// `UntypedSamplerHandleType`) to their underlying `uint` heap index, enforcing the invariant that
/// no untyped handle survives to emit. The `ResourceDescriptorHeap[i]` / `SamplerDescriptorHeap[j]`
/// subscript produces a `uint` index wrapped in an untyped handle; an implicit conversion later
/// unwraps it into a `DescriptorHandle<T>`. On the common path the wrap/unwrap pair is adjacent and
/// collapsed by peephole, but a handle first stored in a local `var` (or passed/returned) leaves
/// the type behind. This pass forwards every
/// `Cast*UntypedResourceHandle*`/`Cast*UntypedSamplerHandle*` (both sides are `uint`, so each is an
/// identity) and rewrites the two type ops to `uint`, so emit and layout never observe an untyped
/// handle. Runs unconditionally for every target.
void lowerUntypedResourceHandleToUInt(IRModule* module);

} // namespace Slang
