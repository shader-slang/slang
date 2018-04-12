// ir-validate.cpp
#include "ir-validate.h"

#include "ir.h"
#include "ir-insts.h"

namespace Slang
{
    struct IRValidateContext
    {
        // The IR module we are validating.
        IRModule*           module;

        // A diagnostic sink to send errors to if anything is invalid.
        DiagnosticSink*     sink;

        DiagnosticSink* getSink() { return sink; }

        // A set of instructions we've seen, to help confirm that
        // values are defined before they are used in a given block.
        HashSet<IRInst*>    seenInsts;
    };

    void validateIRInst(
        IRValidateContext*  context,
        IRInst*             inst);

    void validate(IRValidateContext* context, bool condition, IRInst* inst, char const* message)
    {
        if (!condition)
        {
            context->getSink()->diagnose(inst, Diagnostics::irValidationFailed, message);
        }
    }

    void validateIRInstChildren(
        IRValidateContext*  context,
        IRParentInst*       parent)
    {
        IRInst* prevChild = nullptr;
        for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
        {
            // We need to check the integrity of the parent/next/prev links of
            // all of our instructions
            validate(context, child->parent == parent,  child, "parent link");
            validate(context, child->prev == prevChild, child, "next/prev link");

            // Recursively validate the instruction itself.
            validateIRInst(context, child);

            prevChild = child;
        }
    }

    void validateIRInstOperand(
        IRValidateContext*  context,
        IRInst*             inst,
        IRUse*              operandUse)
    {
        // The `IRUse` for the operand had better have `inst` as its user.
        validate(context, operandUse->getUser() == inst, inst, "operand user");

        // The value we are using needs to fit into one of a few cases.
        //
        // * If the parent of `inst` and of `operand` is the same block, then
        //   we require that `operand` is defined before `inst`
        //
        // * If the parents of `inst` and `operand` are both blocks in the
        //   same functin, then the block defining `operand` must dominate
        //   the block defining `inst`.
        //
        // * Otherwise, we simply require that the parent of `operand` be
        //   an ancestor (transitive parent) of `inst`.

        auto instParent = inst->getParent();

        auto operandValue = operandUse->get();
        auto operandParent = operandValue->getParent();

        if (auto instParentBlock = as<IRBlock>(instParent))
        {
            if (auto operandParentBlock = as<IRBlock>(operandParent))
            {
                if (instParentBlock == operandParentBlock)
                {
                    // If `operandValue` precedes `inst`, then we should
                    // have already seen it, because we scan parent instructions
                    // in order.
                    validate(context, context->seenInsts.Contains(operandValue),    inst, "def must come before use in same block");
                    return;
                }

                auto instFunc = instParentBlock->getParent();
                auto operandFunc = operandParentBlock->getParent();
                if (instFunc == operandFunc)
                {
                    // The two instructions are defined in different blocks of
                    // the same function (or another value with code). We need
                    // to validate that `operandParentBlock` dominates `instParentBlock`.
                    //
                    // TODO: implement this validation once we compute dominator trees.
                    //
                    // validate(context, operandParentBlock->dominates(instParentBlock),    inst, "def must dominate use");
                    return;
                }
            }
        }

        // If the special cases above did not trigger, then either the two values
        // are nested in the same parent, but that parent isn't a block, or they
        // are nested in distinct parents, and those parents aren't both children
        // of a function.
        //
        // In either case, we need to enforce that the parent of `operand` needs
        // to be an ancestor of `inst`.
        //
        for (auto pp = instParent; pp; pp = pp->getParent())
        {
            if (pp == operandParent)
                return;
        }
        //
        // We failed to find `operandParent` while walking the ancestors of `inst`,
        // so something had gone wrong.
        validate(context, false, inst, "def must be ancestor of use");
    }

    void validateIRInstOperands(
        IRValidateContext*  context,
        IRInst*             inst)
    {
        if(inst->getFullType())
            validateIRInstOperand(context, inst, &inst->typeUse);

        UInt operandCount = inst->getOperandCount();
        for (UInt ii = 0; ii < operandCount; ++ii)
        {
            validateIRInstOperand(context, inst, inst->getOperands() + ii);
        }
    }

    void validateIRInst(
        IRValidateContext*  context,
        IRInst*             inst)
    {
        // Validate that any operands of the instruction are used appropriately
        validateIRInstOperands(context, inst);
        context->seenInsts.Add(inst);

        // If `inst` is itself a parent instruction, then we need to recursively
        // validate its children.
        if (auto parent = as<IRParentInst>(inst))
        {
            validateIRInstChildren(context, parent);
        }
    }

    void validateIRModule(IRModule* module, DiagnosticSink* sink)
    {
        IRValidateContext contextStorage;
        IRValidateContext* context = &contextStorage;
        context->module = module;
        context->sink = sink;

        auto moduleInst = module->moduleInst;

        validate(context, moduleInst != nullptr,            moduleInst, "module instruction");
        validate(context, moduleInst->parent == nullptr,    moduleInst, "module instruction parent");
        validate(context, moduleInst->prev == nullptr,      moduleInst, "module instruction prev");
        validate(context, moduleInst->next == nullptr,      moduleInst, "module instruction next");

        validateIRInst(context, module->moduleInst);
    }

    void validateIRModuleIfEnabled(
        CompileRequest* compileRequest,
        IRModule*       module)
    {
        if (!compileRequest->shouldValidateIR)
            return;

        auto sink = &compileRequest->mSink;
        validateIRModule(module, sink);
    }
}
