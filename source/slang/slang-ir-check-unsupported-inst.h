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

/// Diagnose `T*` values made from an integer or from `nullptr`, for targets without pointers to
/// memory addresses (the D3D and WebGPU targets).
///
/// Call after specialization and inlining, so that each remaining value is reported once at the
/// code that makes it. Pointers read from memory are not checked.
void checkUnsupportedPhysicalPointers(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
