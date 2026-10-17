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

/// Diagnose physical pointers (`T*`, a `Ptr` in the `UserPointer` address space, which holds a
/// memory address) on WGSL and the D3D targets, which have none. Reported where a pointer enters
/// the program: a cast from an integer address, `nullptr`, a pointer (or a struct or array
/// containing one) loaded from memory, a global parameter, or an entry-point parameter. Each
/// source location is reported once.
///
/// This validation is required regardless of the selected optimization mode.
void checkUnsupportedPhysicalPointers(
    IRModule* module,
    TargetRequest* target,
    DiagnosticSink* sink);

/// Diagnose `BFloat16` values (scalars, vectors and matrices, also inside structs and arrays) on
/// WGSL, which has no bfloat16 type, where they enter the program, in the same way as
/// `checkUnsupportedPhysicalPointers`.
///
/// This validation is required regardless of the selected optimization mode.
void checkUnsupportedBFloat16(IRModule* module, TargetRequest* target, DiagnosticSink* sink);
} // namespace Slang
