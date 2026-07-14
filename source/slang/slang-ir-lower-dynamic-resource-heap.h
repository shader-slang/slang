#pragma once

namespace Slang
{

struct IRModule;
class TargetProgram;
class DiagnosticSink;

/// Replace `GetDynamicResourceHeap` insts with an actual array of resources.
void lowerDynamicResourceHeap(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

/// Lower any surviving untyped descriptor-heap handle (`UntypedResourceHandleType` /
/// `UntypedSamplerHandleType`) to its underlying `uint` heap index, so emit and layout never
/// observe an untyped handle.
void lowerUntypedResourceHandleToUInt(IRModule* module);

} // namespace Slang
