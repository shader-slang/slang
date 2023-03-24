// slang-ir-validate.h
#pragma once

namespace Slang
{
    struct CodeGenContext;
    class CompileRequestBase;
    class DiagnosticSink;
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
    void validateIRModuleIfEnabled(
        CompileRequestBase* compileRequest,
        IRModule*           module);

    void validateIRModuleIfEnabled(
        CodeGenContext* codeGenContext,
        IRModule*       module);

    void disableIRValidationAtInsert();
    void enableIRValidationAtInsert();

}
