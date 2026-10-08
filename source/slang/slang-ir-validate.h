// slang-ir-validate.h
#pragma once

namespace Slang
{
enum class CodeGenTarget;
struct CodeGenContext;
class CompileRequestBase;
class DiagnosticSink;
class TargetRequest;
struct IRModule;
struct IRInst;

// Validate that an IR module obeys the invariants we need to enforce.
// For example:
//
// * Confirm that linked lists for children and for use-def chains are consistent
//   (e.g., x.next.prev == x)
//
// * Confirm that parent/child relationships are correct (e.g., if is `x` is in
//   `y.children`, then `x.parent == y`
//
// * Confirm that every operand of an instruction is valid to reference (i.e., it
//   must either be defined earlier in the same block, in a different block that
//   dominates the current one, or in a parent instruction of the block.
//
// * Confirm that every block ends with a terminator, and there are no terminators
//   elsewhere in a block.
//
// * Confirm that all the parameters of a block come before any "ordinary" instructions.
void validateIRModule(IRModule* module, DiagnosticSink* sink);
void validateIRInst(IRInst* inst);

// A wrapper that calls `validateIRModule` only when IR validation is enabled
// for the given compile request.
void validateIRModuleIfEnabled(CompileRequestBase* compileRequest, IRModule* module);

void validateIRModuleIfEnabled(CodeGenContext* codeGenContext, IRModule* module);

// RAII class to manage IR validation state in an exception-safe manner
class [[nodiscard]] IRValidationScope
{
public:
    // Constructor saves current state and sets new state
    explicit IRValidationScope(bool enableValidation);

    // Destructor automatically restores previous state
    ~IRValidationScope();

    // Non-copyable to prevent accidental copies
    IRValidationScope(const IRValidationScope&) = delete;
    IRValidationScope& operator=(const IRValidationScope&) = delete;

    // Non-movable to keep it simple
    IRValidationScope(IRValidationScope&&) = delete;
    IRValidationScope& operator=(IRValidationScope&&) = delete;

private:
    bool m_previousState;
};

// Convenience functions to create scoped guards
[[nodiscard]] inline IRValidationScope enableIRValidationScope()
{
    return IRValidationScope(true);
}

[[nodiscard]] inline IRValidationScope disableIRValidationScope()
{
    return IRValidationScope(false);
}

// Validate that the destination of an atomic operation is appropriate, meaning it's
// either 'groupshared' or in a device buffer.
// Note that validation of atomic operations should be done after address space
// specialization for targets (e.g. SPIR-V and Metal) which support this kind of use-case:
//   void atomicOp(inout int array){ InterlockedAdd(array, 1);}
//   groupshared int gArray;
//   [numthreads(1, 1, 1)] void main() { atomicOp(gArray); }
// If 'skipFuncParamValidation' is true, then the validation allows destinations that
// lead back to in/inout parameters that we can't validate.
//
// The memory order operands are also validated against the operation (e.g. a load cannot
// release). Orders must be compile-time constants only on targets that encode them (SPIR-V and
// Metal); other targets ignore the order, so a non-constant order is accepted there.
void validateAtomicOperations(
    bool skipFuncParamValidation,
    CodeGenTarget target,
    DiagnosticSink* sink,
    IRInst* inst);

// Overload that takes IRModule* first for use with SLANG_PASS macro. Returns false if any atomic
// operation in the module was diagnosed as an error.
bool validateAtomicOperations(
    IRModule* module,
    bool skipFuncParamValidation,
    CodeGenTarget target,
    DiagnosticSink* sink);

void validateVectorsAndMatrices(
    IRModule* module,
    DiagnosticSink* sink,
    TargetRequest* targetRequest);

/// Reject mutable globals whose linked types require unsupported storage.
///
/// Requires a linked `module` at the pipeline checkpoint after `specializeModule` and before
/// `legalizeResourceTypes`. Checks file- or namespace-scope `static` variables and uniform
/// shadows marked with `IRFileOrNamespaceScopeStaticVarDecoration`. Returns `false` after
/// diagnosing any such variable that contains opaque values or unsized arrays by value.
bool validateMutableGlobalVariableTypes(IRModule* module, DiagnosticSink* sink);

bool validateStructuredBufferResourceTypes(
    IRModule* module,
    DiagnosticSink* sink,
    TargetRequest* targetRequest);

// Process kIROp_AssumeAddress instructions. When validate is true, checks that
// getRootAddr(addr) is not a function-local variable (kIROp_Var) holding a
// plain value, and emits an error if it is. Vars whose stored type is a pointer,
// pointer-like resource (e.g. ConstantBuffer), structured buffer, or
// byte-address buffer are exempt because those always refer to device memory.
// Always replaces each AssumeAddress(x) with x so backend passes never see the
// opcode.
void validateAndRemoveAssumeAddress(IRModule* module, bool validate, DiagnosticSink* sink);

} // namespace Slang
