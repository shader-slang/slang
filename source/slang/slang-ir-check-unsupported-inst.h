#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetRequest;

/// Diagnose instructions and types that the target cannot represent. The checks run at the end of
/// `linkAndOptimizeIR`, after address-space specialization, so they see the IR that the emitter
/// receives. They only reject input: a shader that passes them compiles the same way when they are
/// skipped, as they are under `-minimum-slang-optimization`.
void checkUnsupportedInst(IRModule* module, TargetRequest* target, DiagnosticSink* sink);

/// Diagnose resource types that cannot be represented in mutable local storage for compiled D3D
/// or Metal output.
///
/// Call after resource specialization and target lowering, including passes that create locals.
/// This validation is required regardless of the selected optimization mode. Other targets,
/// including HLSL source output, are not checked by this operation.
void checkUnsupportedResourceLocals(IRModule* module, TargetRequest* target, DiagnosticSink* sink);

/// Diagnose atomic operations for CPU-via-LLVM targets. The LLVM backend implements
/// no IR atomic operation, so an atomic reaching the emitter aborts code generation with an
/// internal error. Call after `validateAtomicOperations` so destination validation still runs
/// first. This validation is required regardless of the selected optimization mode.
void checkUnsupportedLLVMCPUAtomics(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
