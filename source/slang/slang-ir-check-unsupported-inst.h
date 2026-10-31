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
/// memory address) on the D3D targets (HLSL, DXBC, DXIL) and the WebGPU targets (WGSL, also when
/// compiled on to SPIR-V), which cannot represent them. A program that would emit a physical
/// pointer is diagnosed, so it does not reach the emitter. Each pointer value is reported once,
/// where it enters the program (not again where it is passed to a function or stored and read
/// back), and each source location once.
///
/// This must run after specialization, inlining, phi elimination and address-space
/// specialization, and after uniform parameters are moved into constant buffers. It is required
/// regardless of the selected optimization mode.
void checkUnsupportedPhysicalPointers(
    IRModule* module,
    TargetRequest* target,
    DiagnosticSink* sink);

/// Diagnose `BFloat16` values (scalars, vectors and matrices, also inside structs and arrays) on
/// the WebGPU targets, which have no bfloat16 type. A program that would emit a `BFloat16` is
/// diagnosed, so it does not reach the emitter. Each value is reported once, where it enters the
/// program, and each source location once.
///
/// The pipeline position is the same as for `checkUnsupportedPhysicalPointers`, and this is also
/// required regardless of the selected optimization mode.
void checkUnsupportedBFloat16(IRModule* module, TargetRequest* target, DiagnosticSink* sink);
} // namespace Slang
