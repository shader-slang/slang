#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetRequest;

void checkUnsupportedInst(IRModule* module, TargetRequest* target, DiagnosticSink* sink);

/// Diagnose resource types that cannot be represented in mutable local storage for compiled D3D
/// or Metal output.
///
/// Call after resource specialization and target lowering, including passes that create locals.
/// This validation is required regardless of the selected optimization mode. Other targets,
/// including HLSL source output, are not checked by this operation.
void checkUnsupportedResourceLocals(IRModule* module, TargetRequest* target, DiagnosticSink* sink);

/// Diagnose `T*` values (a `Ptr` in the `UserPointer` address space) that code makes from an
/// integer or from `nullptr`. The caller runs this only for targets without such pointers (D3D
/// and WebGPU); the check itself reports unconditionally.
///
/// Call it last, after the passes that can still create these values (for example the peephole
/// that turns `if (p)` into `p != nullptr`). Pointers read from memory are not checked; a pointer
/// unpacked from an existential (AnyValue) value is made with `BitCast` and is reported.
void checkUnsupportedPhysicalPointers(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
