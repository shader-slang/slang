// slang-ir-validate.cpp
#include "slang-ir-validate.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

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
        IRInst*             parent)
    {
        // We want to check that child instructions are correctly
        // ordered so that decorations come first, then any parameters,
        // and then any ordinary instructions.
        //
        // We will track what we have seen so far with a simple state
        // machine, which in valid IR should proceed monitonically
        // up through the following states:
        //
        enum State
        {
            kState_Initial = 0,
            kState_AfterDecoration,
            kState_AfterParam,
            kState_AfterOrdinary,
        };
        State state = kState_Initial;

        IRInst* prevChild = nullptr;
        for(auto child : parent->getDecorationsAndChildren() )
        {
            // We need to check the integrity of the parent/next/prev links of
            // all of our instructions
            validate(context, child->parent == parent,  child, "parent link");
            validate(context, child->prev == prevChild, child, "next/prev link");

            // Recursively validate the instruction itself.
            validateIRInst(context, child);

            if( as<IRDecoration>(child) )
            {
                validate(context, state <= kState_AfterDecoration, child, "decorations must come before other child instructions");
                state = kState_AfterDecoration;
            }
            else if( as<IRParam>(child) )
            {
                validate(context, state <= kState_AfterParam, child, "parameters must come before ordinary instructions");
                state = kState_AfterParam;
            }
            else
            {
                state = kState_AfterOrdinary;
            }

            // Do some extra validation around terminator instructions:
            //
            // * The last instruction of a block should always be a terminator
            // * No other instruction should be a terminator
            //
            if(as<IRBlock>(parent) && (child == parent->getLastDecorationOrChild()))
            {
                validate(context, as<IRTerminatorInst>(child) != nullptr, child, "last instruction in block must be terminator");
            }
            else
            {
                validate(context, !as<IRTerminatorInst>(child), child, "terminator must be last instruction in a block");
            }


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

        if( !operandValue )
        {
            // A null operand should almost always be an error, but
            // we currently have a few cases where this arises.
            //
            // TODO: plug the leaks.
            return;
        }

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

    void validateCodeBody(IRValidateContext* context, IRGlobalValueWithCode* code)
    {
        HashSet<IRBlock*> blocks;
        for (auto block : code->getBlocks())
            blocks.Add(block);
        auto validateBranchTarget = [&](IRInst* inst, IRBlock* target)
        {
            validate(
                context,
                blocks.Contains(target),
                inst,
                "branch inst must have a valid target block that is defined within the same "
                "scope.");
        };
        for (auto block : code->getBlocks())
        {
            auto terminator = block->getTerminator();
            validate(context, terminator, block, "block must have valid terminator inst.");
            switch (terminator->getOp())
            {
            case kIROp_conditionalBranch:
                validateBranchTarget(
                    terminator, as<IRConditionalBranch>(terminator)->getTrueBlock());
                validateBranchTarget(
                    terminator, as<IRConditionalBranch>(terminator)->getFalseBlock());
                break;
            case kIROp_loop:
            case kIROp_unconditionalBranch:
                validateBranchTarget(terminator, as<IRUnconditionalBranch>(terminator)->getTargetBlock());
                break;
            case kIROp_Switch:
                {
                    auto switchInst = as<IRSwitch>(terminator);
                    for (UInt i = 0; i < switchInst->getCaseCount(); i++)
                    {
                        validateBranchTarget(switchInst, switchInst->getCaseLabel(i));
                    }
                    validateBranchTarget(switchInst, switchInst->getDefaultLabel());
                    validateBranchTarget(switchInst, switchInst->getBreakLabel());
                }
            }
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
        validateIRInstChildren(context, inst);

        if (auto code = as<IRGlobalValueWithCode>(inst))
        {
            validateCodeBody(context, code);
        }
    }

    void validateIRModule(IRModule* module, DiagnosticSink* sink)
    {
        IRValidateContext contextStorage;
        IRValidateContext* context = &contextStorage;
        context->module = module;
        context->sink = sink;

        auto moduleInst = module->getModuleInst();

        validate(context, moduleInst != nullptr,            moduleInst, "module instruction");
        validate(context, moduleInst->parent == nullptr,    moduleInst, "module instruction parent");
        validate(context, moduleInst->prev == nullptr,      moduleInst, "module instruction prev");
        validate(context, moduleInst->next == nullptr,      moduleInst, "module instruction next");

        validateIRInst(context, moduleInst);
    }

    void validateIRModuleIfEnabled(
        CompileRequestBase*  compileRequest,
        IRModule*               module)
    {
        if (!compileRequest->shouldValidateIR)
            return;

        auto sink = compileRequest->getSink();
        validateIRModule(module, sink);
    }

    void validateIRModuleIfEnabled(
        CodeGenContext* codeGenContext,
        IRModule* module)
    {
        if (!codeGenContext->shouldValidateIR())
            return;

        auto sink = codeGenContext->getSink();
        validateIRModule(module, sink);
    }
}
