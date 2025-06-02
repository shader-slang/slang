// slang-ir-validate.cpp
#include "slang-ir-validate.h"

#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct IRValidateContext
{
    // The IR module we are validating.
    IRModule* module;

    RefPtr<IRDominatorTree> domTree;

    // A diagnostic sink to send errors to if anything is invalid.
    DiagnosticSink* sink;

    DiagnosticSink* getSink() { return sink; }

    // A set of instructions we've seen, to help confirm that
    // values are defined before they are used in a given block.
    HashSet<IRInst*> seenInsts;
};

void validateIRInst(IRValidateContext* context, IRInst* inst);

void validate(IRValidateContext* context, bool condition, IRInst* inst, char const* message)
{
    if (!condition)
    {
        if (context)
        {
            context->getSink()->diagnose(inst, Diagnostics::irValidationFailed, message);
        }
        else
        {
            SLANG_ASSERT_FAILURE("IR validation failed");
        }
    }
}

void validateIRInstChildren(IRValidateContext* context, IRInst* parent)
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
    bool hasSeenTerminatorInst = false;
    for (auto child : parent->getDecorationsAndChildren())
    {
        // We need to check the integrity of the parent/next/prev links of
        // all of our instructions
        validate(context, child->parent == parent, child, "parent link");
        validate(context, child->prev == prevChild, child, "next/prev link");

        // Recursively validate the instruction itself.
        validateIRInst(context, child);

        if (as<IRDecoration>(child))
        {
            validate(
                context,
                state <= kState_AfterDecoration,
                child,
                "decorations must come before other child instructions");
            state = kState_AfterDecoration;
        }
        else if (as<IRParam, IRDynamicCastBehavior::NoUnwrap>(child))
        {
            validate(
                context,
                state <= kState_AfterParam,
                child,
                "parameters must come before ordinary instructions");
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
        if (as<IRBlock>(parent) && (child == parent->getLastDecorationOrChild()))
        {
            validate(
                context,
                as<IRTerminatorInst>(child) != nullptr,
                child,
                "last instruction in block must be terminator");
        }
        else
        {
            validate(
                context,
                !as<IRTerminatorInst>(child),
                child,
                "terminator must be last instruction in a block");
        }

        if (as<IRTerminatorInst>(child))
        {
            validate(
                context,
                !hasSeenTerminatorInst,
                child,
                "block must not contain more than one terminator");
            hasSeenTerminatorInst = true;
        }
        prevChild = child;
    }
}

void validateIRInstOperand(IRValidateContext* context, IRInst* inst, IRUse* operandUse)
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

    if (!operandValue)
    {
        // A null operand should almost always be an error, but
        // we currently have a few cases where this arises.
        //
        // TODO: plug the leaks.
        return;
    }

    auto operandParent = operandValue->getParent();

    auto instParentBlock = getBlock(inst);
    if (instParentBlock)
    {
        if (auto operandParentBlock = as<IRBlock>(operandParent))
        {
            if (instParentBlock == operandParentBlock)
            {
                // If `operandValue` precedes `inst`, then we should
                // have already seen it, because we scan parent instructions
                // in order.
                if (context)
                {
                    validate(
                        context,
                        context->seenInsts.contains(operandValue),
                        inst,
                        "def must come before use in same block");
                }
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
                if (context && context->domTree)
                {
                    validate(
                        context,
                        context->domTree->dominates(operandParentBlock, instParentBlock),
                        inst,
                        "def must dominate use");
                }
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

    // We allow out-of-order def-use in global scope.
    bool allInGlobalScope = inst->getParent() && inst->getParent()->getOp() == kIROp_Module;
    if (allInGlobalScope)
    {
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            auto op = inst->getOperand(i);
            if (!op)
                continue;
            if (!op->getParent())
                continue;
            if (op->getParent()->getOp() != kIROp_Module)
            {
                allInGlobalScope = false;
                break;
            }
        }
    }
    if (allInGlobalScope)
        return;

    // Allow exceptions.
    switch (inst->getOp())
    {
    case kIROp_DifferentiableTypeDictionaryItem:
    case kIROp_DebugScope:
        return;
    }
    //
    // We failed to find `operandParent` while walking the ancestors of `inst`,
    // so something had gone wrong.
    validate(context, false, inst, "def must be ancestor of use");
}

void validateIRInstOperands(IRValidateContext* context, IRInst* inst)
{
    if (inst->getFullType())
        validateIRInstOperand(context, inst, &inst->typeUse);

    // Avoid validating decoration operands
    // since they don't have to conform to inst visibility
    // constraints.
    //
    if (as<IRDecoration>(inst))
        return;

    UInt operandCount = inst->getOperandCount();
    for (UInt ii = 0; ii < operandCount; ++ii)
    {
        validateIRInstOperand(context, inst, inst->getOperands() + ii);
    }
}

static thread_local bool _enableIRValidationAtInsert = false;
void disableIRValidationAtInsert()
{
    _enableIRValidationAtInsert = false;
}
void enableIRValidationAtInsert()
{
    _enableIRValidationAtInsert = true;
}
void validateIRInstOperands(IRInst* inst)
{
    if (!_enableIRValidationAtInsert)
        return;
    switch (inst->getOp())
    {
    case kIROp_loop:
    case kIROp_ifElse:
    case kIROp_unconditionalBranch:
    case kIROp_conditionalBranch:
    case kIROp_Switch:
        return;
    default:
        break;
    }

    validateIRInstOperands(nullptr, inst);
}

void validateCodeBody(IRValidateContext* context, IRGlobalValueWithCode* code)
{
    HashSet<IRBlock*> blocks;
    for (auto block : code->getBlocks())
        blocks.add(block);
    auto validateBranchTarget = [&](IRInst* inst, IRBlock* target)
    {
        validate(
            context,
            blocks.contains(target),
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
            validateBranchTarget(terminator, as<IRConditionalBranch>(terminator)->getTrueBlock());
            validateBranchTarget(terminator, as<IRConditionalBranch>(terminator)->getFalseBlock());
            break;
        case kIROp_loop:
        case kIROp_unconditionalBranch:
            validateBranchTarget(
                terminator,
                as<IRUnconditionalBranch>(terminator)->getTargetBlock());
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

void validateIRInst(IRValidateContext* context, IRInst* inst)
{
    // Validate that any operands of the instruction are used appropriately
    validateIRInstOperands(context, inst);
    context->seenInsts.add(inst);

    if (auto code = as<IRGlobalValueWithCode>(inst))
    {
        context->domTree = computeDominatorTree(code);
        validateCodeBody(context, code);
    }

    // If `inst` is itself a parent instruction, then we need to recursively
    // validate its children.
    validateIRInstChildren(context, inst);

    if (as<IRGlobalValueWithCode>(inst))
        context->domTree = nullptr;
}

void validateIRInst(IRInst* inst)
{
    IRValidateContext contextStorage;
    IRValidateContext* context = &contextStorage;
    DiagnosticSink sink;
    context->module = inst->getModule();
    context->sink = &sink;
    if (auto func = as<IRFunc>(inst))
        context->domTree = computeDominatorTree(func);
    validateIRInst(context, inst);
}

void validateIRModule(IRModule* module, DiagnosticSink* sink)
{
    IRValidateContext contextStorage;
    IRValidateContext* context = &contextStorage;
    context->module = module;
    context->sink = sink;

    auto moduleInst = module->getModuleInst();

    validate(context, moduleInst != nullptr, moduleInst, "module instruction");
    validate(context, moduleInst->parent == nullptr, moduleInst, "module instruction parent");
    validate(context, moduleInst->prev == nullptr, moduleInst, "module instruction prev");
    validate(context, moduleInst->next == nullptr, moduleInst, "module instruction next");

    validateIRInst(context, moduleInst);
}

void validateIRModuleIfEnabled(CompileRequestBase* compileRequest, IRModule* module)
{
    if (!compileRequest->getLinkage()->m_optionSet.getBoolOption(CompilerOptionName::ValidateIr))
        return;

    auto sink = compileRequest->getSink();
    validateIRModule(module, sink);
}

void validateIRModuleIfEnabled(CodeGenContext* codeGenContext, IRModule* module)
{
    if (!codeGenContext->shouldValidateIR())
        return;

    auto sink = codeGenContext->getSink();
    validateIRModule(module, sink);
}

// Returns whether 'dst' is a valid destination for atomic operations, meaning
// it leads either to 'groupshared' or 'device buffer' memory.
static bool isValidAtomicDest(bool skipFuncParamValidation, IRInst* dst)
{
    bool isGroupShared = as<IRGroupSharedRate>(dst->getRate());
    if (isGroupShared)
        return true;

    if (as<IRRWStructuredBufferGetElementPtr>(dst))
        return true;
    if (as<IRImageSubscript>(dst))
        return true;

    if (auto ptrType = as<IRPtrType>(dst->getDataType()))
    {
        switch (ptrType->getAddressSpace())
        {
        case AddressSpace::Global:
        case AddressSpace::GroupShared:
        case AddressSpace::StorageBuffer:
        case AddressSpace::UserPointer:
            return true;
        default:
            break;
        }
    }

    if (as<IRGlobalParam>(dst))
    {
        switch (dst->getDataType()->getOp())
        {
        case kIROp_GLSLShaderStorageBufferType:
        case kIROp_TextureType:
            return true;
        default:
            return false;
        }
    }

    if (auto param = as<IRParam>(dst))
    {
        auto paramType = param->getDataType();
        if (auto outType = as<IROutTypeBase>(paramType))
        {
            if (outType->getAddressSpace() == AddressSpace::GroupShared)
            {
                return true;
            }
            else if (skipFuncParamValidation)
            {
                // We haven't actually verified that this is a valid atomic operation destination,
                // but the callee wants to skip this specific validation.
                return true;
            }
        }
    }
    if (auto getElementPtr = as<IRGetElementPtr>(dst))
        return isValidAtomicDest(skipFuncParamValidation, getElementPtr->getBase());
    if (auto getOffsetPtr = as<IRGetOffsetPtr>(dst))
        return isValidAtomicDest(skipFuncParamValidation, getOffsetPtr->getBase());
    if (auto fieldAddress = as<IRFieldAddress>(dst))
        return isValidAtomicDest(skipFuncParamValidation, fieldAddress->getBase());

    return false;
}

void validateAtomicOperations(bool skipFuncParamValidation, DiagnosticSink* sink, IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_AtomicLoad:
    case kIROp_AtomicStore:
    case kIROp_AtomicExchange:
    case kIROp_AtomicCompareExchange:
    case kIROp_AtomicAdd:
    case kIROp_AtomicSub:
    case kIROp_AtomicAnd:
    case kIROp_AtomicOr:
    case kIROp_AtomicXor:
    case kIROp_AtomicMin:
    case kIROp_AtomicMax:
    case kIROp_AtomicInc:
    case kIROp_AtomicDec:
        {
            IRInst* destinationPtr = inst->getOperand(0);
            if (!isValidAtomicDest(skipFuncParamValidation, destinationPtr))
                sink->diagnose(inst->sourceLoc, Diagnostics::invalidAtomicDestinationPointer);
        }
        break;

    default:
        break;
    }

    for (auto child : inst->getModifiableChildren())
    {
        validateAtomicOperations(skipFuncParamValidation, sink, child);
    }
}

static void validateVectorOrMatrixElementType(
    DiagnosticSink* sink,
    SourceLoc sourceLoc,
    IRType* elementType,
    uint32_t allowedWidths,
    const DiagnosticInfo& disallowedElementTypeEncountered)
{
    if (!isFloatingType(elementType))
    {
        if (isIntegralType(elementType))
        {
            IntInfo info = getIntTypeInfo(elementType);
            if (allowedWidths == 0U)
            {
                sink->diagnose(sourceLoc, disallowedElementTypeEncountered, elementType);
            }
            else
            {
                bool widthAllowed = false;
                SLANG_ASSERT((allowedWidths & ~(0xfU << 3)) == 0U);
                for (uint32_t p = 3U; p <= 6U; p++)
                {
                    uint32_t width = 1U << p;
                    if (!(allowedWidths & width))
                        continue;
                    widthAllowed = widthAllowed || (info.width == width);
                }
                if (!widthAllowed)
                {
                    sink->diagnose(sourceLoc, disallowedElementTypeEncountered, elementType);
                }
            }
        }
        else if (!as<IRBoolType>(elementType))
        {
            sink->diagnose(sourceLoc, disallowedElementTypeEncountered, elementType);
        }
    }
}

static void validateVectorElementCount(DiagnosticSink* sink, IRVectorType* vectorType)
{
    const auto elementCount = as<IRIntLit>(vectorType->getElementCount())->getValue();

    // 1-vectors are supported and are legalized/transformed properly when targetting unsupported
    // backends.
    const IRIntegerValue minCount = 1;
    const IRIntegerValue maxCount = 4;
    if ((elementCount < minCount) || (elementCount > maxCount))
    {
        sink->diagnose(
            vectorType->sourceLoc,
            Diagnostics::vectorWithInvalidElementCountEncountered,
            elementCount,
            "1",
            maxCount);
    }
}

struct ValidateDynAndSomeUsage
{
    // fail point is the "first block which when followed causes a failiure",
    // not the write which causes a failiure (that may be multiple writes).
    IRInst* failPoint = nullptr;
    List<IRInst*> potentialIssues = List<IRInst*>();
    HashSet<IRBlock*> blocksToSkip = HashSet<IRBlock*>();
    void resetChecks()
    {
        blockTraversalCache = Dictionary<IRInst*, int>();
        failPoint = nullptr;
        potentialIssues = {};
        blocksToSkip = {};
    }
    int totalAssignmentsToVarGivenInst(IRInst* var, IRInst* instToCheck)
    {
        switch (instToCheck->getOp())
        {
        case kIROp_Store:
            {
                // always considered 1 assignment
                IRStore* storeOp = as<IRStore>(instToCheck);
                if (storeOp->getPtr() != var)
                    return 0;
                potentialIssues.add(instToCheck);
                return 1;
            }
        case kIROp_Call:
            {
                // If `var` is an `out` param, we need to add 1 to the total assignments
                IRCall* callOp = as<IRCall>(instToCheck);
                IRFunc* calleeFunc = as<IRFunc>(callOp->getCallee());
                if (!calleeFunc)
                    return 0;
                auto count = -1;
                for (auto arg : callOp->getArgsList())
                {
                    count++;
                    if (var != arg)
                        continue;
                    break;
                }
                if (as<IROutType>(calleeFunc->getParamType(count)))
                {
                    potentialIssues.add(instToCheck);
                    return 1;
                }
                return 0;
            }
        case kIROp_ifElse:
            {
                // Store the max total assignments in the if-else block.
                // We assume worst case, so if 1 block has 1 assignment
                // and rest have 0, we assume the entire block to have 1 assignment.

                /*
                we need to step through a mini cfg since `uses` does not store which if-else-chain a 
                inst comes from; this is important because 1 write inside an if and 1 write inside an else block 
                is considered 1 write total
                */

                IRIfElse* ifElseOp = as<IRIfElse>(instToCheck);                
                int total = 0;

                List<IRInst*> blocksToIterOn = List<IRInst*>();
                blocksToIterOn.reserve(2);
                blocksToIterOn.add(ifElseOp->getTrueBlock());
                blocksToIterOn.add(ifElseOp->getFalseBlock());

                for (auto maybeBlock : blocksToIterOn)
                {
                    if (!maybeBlock)
                        continue;

                    int totalAssignments = collectAssignmentsInBlock(
                        var,
                        as<IRBlock>(maybeBlock));
                    if (failPoint)
                        return 0;
                    total = (total > totalAssignments) ? total : totalAssignments;
                }
                return total;
            }
        case kIROp_loop:
            {
                IRLoop* loopOp = as<IRLoop>(instToCheck);
                // We can assume iterators can always be changed if loops
                // are not-unrollable, therefore if this loop runs with an
                // assignment we must fail.
                //
                // part of loop logic is TargetBlock->BreakBlock. This is an
                // issue since the BreakBlock is outside the loop, so we need to
                // visit the BreakBlock first to ensure we can remove the total
                // assignments from the BreakBlock from the for-loop assignment count

                int totalAssignmentsPostLoop =
                    collectAssignmentsInBlock(var, loopOp->getBreakBlock());
                if (failPoint)
                    return 0;
                
                int totalAssignmentsInLoop =
                    collectAssignmentsInBlock(var, loopOp->getTargetBlock()) - totalAssignmentsPostLoop;
                if (totalAssignmentsInLoop > 0)
                    return 2;

                return 0;
            }
        case kIROp_Switch:
            {
                IRSwitch* switchOp = as<IRSwitch>(instToCheck);
                int total = 0;
                for (auto i = 0; i < switchOp->getCaseCount(); i++)
                {
                    auto caseBlock = as<IRBlock>(switchOp->getCaseLabel(i));
                    int totalAssignments =
                        collectAssignmentsInBlock(var, caseBlock);
                    if (failPoint)
                        return 0;
                    total = (total > totalAssignments) ? total : totalAssignments;
                }
                return total;
            }
        case kIROp_unconditionalBranch:
            {
                // just run through all elements of the branch
                IRUnconditionalBranch* branchOp = as<IRUnconditionalBranch>(instToCheck);
                int totalAssignments =
                    collectAssignmentsInBlock(var, branchOp->getTargetBlock());
                if (failPoint)
                    return 0;
                return totalAssignments;
            }
        default:
            return 0;
        }
    }

    // Passes on the fail-point if any.
    // Recursively counts the number of assignments to a var.
    //
    Dictionary<IRInst*, int> blockTraversalCache;
    
    int* getFromCache(IRBlock* inst)
    { 
        return blockTraversalCache.tryGetValue(inst);
    }
    
    void addBlockToCache(IRBlock* inst, int value)
    { 
        blockTraversalCache.add(inst, value);
    }

    bool maybeSkipBlock(IRBlock* inst)
    {
        if (blocksToSkip.contains(inst))
            return true;
        blocksToSkip.add(inst);
        return false;
    }

    int collectAssignmentsInBlock(
        IRInst* var,
        IRBlock* block)
    {
        if (!block)
            return 0;


        // any repeat blocks found need to be fetched from cache
        if (auto cachedTotal = getFromCache(block))
            return *cachedTotal;

        // If we get here, its due to a loop; we are in-process of checking and will populate the cache after the recursion ends
        if (maybeSkipBlock(block))
            return 0;

        int total = 0;
        for (auto inst : block->getChildren())
        {
            total += totalAssignmentsToVarGivenInst(var, inst);

            if (failPoint)
                return 0;
            if (total > 1)
            {
                failPoint = inst;
                return 0;
            }
        }
           
        addBlockToCache(block, total);
        return total;
    }

    // returns nullptr if the var was not written more than once,
    // otherwise returns the inst that wrote it more than once.
    void ensureLocalVarIsWrittenOnce(IRInst* var)
    {
        // Idea here is we need to walk every IRBlock to figure out if we have 2 assignments to our
        // variable in question. We cannot look at IRUse's since these would show 2 `IRStore` in
        // cases with branches without a clear connection between the 2:
        /*
            some Type myVar;
            if(thing[0])
                myVar = Thing();
            else
                myVar = Thing();
        */

        resetChecks();
        auto parentToTraverse = as<IRBlock>(var->getParent());
        collectAssignmentsInBlock(var, parentToTraverse);
    }

    DiagnosticSink* m_sink;
    ValidateDynAndSomeUsage(IRModule* module, DiagnosticSink* sink)
    {
        m_sink = sink;
        for (auto globalInst : module->getGlobalInsts())
        {
            auto func = as<IRFunc>(globalInst);
            if (!func)
                continue;

            for (auto block : func->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    auto op = inst->getOp();
                    if (op != kIROp_Var && op != kIROp_Param)
                        continue;

                    if (!inst->findDecoration<IRSomeTypeDecoration>())
                        continue;

                    // Ensure written once
                    ensureLocalVarIsWrittenOnce(inst);
                    if (failPoint)
                    {
                        sink->diagnose(
                            inst,
                            Diagnostics::localSomeVarWrittenMoreThanOnce,
                            inst);
                        for (auto i : potentialIssues)
                            sink->diagnose(i, Diagnostics::seeUseSite);
                    }

                    if (op != kIROp_Param)
                        continue;

                    // Ensure all possible assignments to `out param` is the same type
                    /*
                    if(...)
                        c = SomeType1();
                    else
                        c = SomeType2(); // invalid, should be SomeType1()
                    */



                }
            }
        }
    }
};

void validateDynAndSomeUsage(IRModule* module, DiagnosticSink* sink)
{
    /*
    Need to validate here the following:
    //Var/Param:
    *. Local var is 100% chance assigned once
    *. Error if using interface-var before initialization
    *. ensure `some` is a concrete type and will NEVER run dynamic dispatch code
    */
    ValidateDynAndSomeUsage(module, sink);
}

void validateVectorsAndMatrices(
    IRModule* module,
    DiagnosticSink* sink,
    TargetRequest* targetRequest)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto matrixType = as<IRMatrixType>(globalInst))
        {
            // Matrices with row/col dimension 1 are only well-supported on D3D targets
            if (!isD3DTarget(targetRequest))
            {
                // Verify that neither row nor col count is 1
                auto colCount = as<IRIntLit>(matrixType->getColumnCount());
                auto rowCount = as<IRIntLit>(matrixType->getRowCount());

                if ((rowCount && (rowCount->getValue() == 1)) ||
                    (colCount && (colCount->getValue() == 1)))
                {
                    sink->diagnose(matrixType->sourceLoc, Diagnostics::matrixColumnOrRowCountIsOne);
                }
            }

            // Verify that the element type is a floating point type, or an allowed integral type
            auto elementType = matrixType->getElementType();
            uint32_t allowedWidths = 0U;
            if (isCPUTarget(targetRequest))
                allowedWidths = 8U | 16U | 32U | 64U;
            else if (isCUDATarget(targetRequest))
                allowedWidths = 32U | 64U;
            else if (isD3DTarget(targetRequest))
                allowedWidths = 16U | 32U;
            validateVectorOrMatrixElementType(
                sink,
                matrixType->sourceLoc,
                elementType,
                allowedWidths,
                Diagnostics::matrixWithDisallowedElementTypeEncountered);
        }
        else if (auto vectorType = as<IRVectorType>(globalInst))
        {
            // Verify that the element type is a floating point type, or an allowed integral type
            auto elementType = vectorType->getElementType();
            uint32_t allowedWidths = 0U;
            if (isWGPUTarget(targetRequest))
                allowedWidths = 32U;
            else
                allowedWidths = 8U | 16U | 32U | 64U;

            validateVectorOrMatrixElementType(
                sink,
                vectorType->sourceLoc,
                elementType,
                allowedWidths,
                Diagnostics::vectorWithDisallowedElementTypeEncountered);

            validateVectorElementCount(sink, vectorType);
        }
    }
}

} // namespace Slang
