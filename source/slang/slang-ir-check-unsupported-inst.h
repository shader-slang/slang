#pragma once

namespace Slang
{
struct IRAtomicOperation;
struct IRModule;
class DiagnosticSink;
class TargetRequest;

/// Diagnose instructions and types that the target cannot represent. The checks run at the end of
/// `linkAndOptimizeIR`, after address-space specialization, so they see the IR that the emitter
/// receives. They only reject input: a shader that passes them compiles the same way when they are
/// skipped, as they are under `-minimum-slang-optimization`.
void checkUnsupportedInst(IRModule* module, TargetRequest* target, DiagnosticSink* sink);

/// Return why MSL cannot express the 64-bit integer atomic `atomic`, or nullptr if it can. MSL has
/// 64-bit atomics only as a min or max that returns void: `atomic_min_explicit` and
/// `atomic_max_explicit` on a device `atomic_ulong` with relaxed order (MSL 4.1 spec, section
/// 6.16.4.6), and the `atomic_min`/`atomic_max` members of a `ulong` texture. For example,
/// `InterlockedMin(buffer[i], key)` on a `RWStructuredBuffer<uint64_t>` fits, but
/// `InterlockedMin(buffer[i], key, original)` does not once `original` is read.
/// `checkUnsupportedInst` reports a non-null reason, and the Metal emitter emits the void form
/// exactly when the reason is null, so a shader compiles the same way when the check is skipped.
const char* getUnsupportedMetal64BitAtomicReason(IRAtomicOperation* atomic);

/// Diagnose resource types that cannot be represented in mutable local storage for compiled D3D
/// or Metal output.
///
/// Call after resource specialization and target lowering, including passes that create locals.
/// This validation is required regardless of the selected optimization mode. Other targets,
/// including HLSL source output, are not checked by this operation.
void checkUnsupportedResourceLocals(IRModule* module, TargetRequest* target, DiagnosticSink* sink);
} // namespace Slang
