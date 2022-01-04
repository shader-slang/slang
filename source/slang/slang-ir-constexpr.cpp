// slang-ir-constexpr.cpp
#include "slang-ir-constexpr.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang {

struct PropagateConstExprContext
{
    IRModule* module;
    IRModule* getModule() { return module; }

    DiagnosticSink* sink;

    SharedIRBuilder sharedBuilder;
    IRBuilder builder;

    List<IRInst*> workList;
    HashSet<IRInst*> onWorkList;

    IRBuilder* getBuilder() { return &builder; }

    Session* getSession() { return sharedBuilder.getSession(); }

    DiagnosticSink* getSink() { return sink; }
};

bool isConstExpr(IRType* fullType)
{
    if( auto rateQualifiedType = as<IRRateQualifiedType>(fullType))
    {
        auto rate = rateQualifiedType->getRate();
        if(auto constExprRate = as<IRConstExprRate>(rate))
            return true;
    }

    return false;
}

bool isConstExpr(IRInst* value)
{
    // Certain IR value ops are implicitly `constexpr`
    //
    // TODO: should we just go ahead and make that explicit
    // in the type system?
    switch(value->getOp())
    {
    case kIROp_IntLit:
    case kIROp_FloatLit:
    case kIROp_BoolLit:
    case kIROp_Func:
        return true;

    default:
        break;
    }

    if(isConstExpr(value->getFullType()))
        return true;

    return false;
}

bool opCanBeConstExpr(IROp op)
{
    switch( op )
    {
    case kIROp_IntLit:
    case kIROp_FloatLit:
    case kIROp_BoolLit:
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_IRem:
    case kIROp_FRem:
    case kIROp_Neg:
    case kIROp_Construct:
    case kIROp_makeVector:
    case kIROp_makeArray:
    case kIROp_MakeMatrix:
    // TODO: more cases
        return true;

    default:
        return false;
    }
}

bool opCanBeConstExpr(IRInst* value)
{
    // TODO: realistically need to special-case `call`
    // operations here, so that we check whether the
    // callee function is fixed/known, and if it is
    // whether it has been decoared as constant-foldable

    return opCanBeConstExpr(value->getOp());
}

void markConstExpr(
    PropagateConstExprContext*  context,
    IRInst*                    value)
{
    Slang::markConstExpr(context->getBuilder(), value);
}


// Propagate `constexpr`-ness in a forward direction, from the
// operands of an instruction to the instruction itself.
bool propagateConstExprForward(
    PropagateConstExprContext*  context,
    IRGlobalValueWithCode*      code)
{
    bool anyChanges = false;
    for(;;)
    {
        bool changedThisIteration = false;
        for( auto bb = code->getFirstBlock(); bb; bb = bb->getNextBlock() )
        {
            for( auto ii = bb->getFirstInst(); ii; ii = ii->getNextInst() )
            {
                // Instruction already `constexpr`? Then skip it.
                if(isConstExpr(ii))
                    continue;

                // Is the operation one that we can actually make be constexpr?
                if(!opCanBeConstExpr(ii))
                    continue;

                // Are all arguments `constexpr`?
                bool allArgsConstExpr = true;
                UInt argCount = ii->getOperandCount();
                for( UInt aa = 0; aa < argCount; ++aa )
                {
                    auto arg = ii->getOperand(aa);

                    if( !isConstExpr(arg) )
                    {
                        allArgsConstExpr = false;
                        break;
                    }
                }
                if(!allArgsConstExpr)
                    continue;

                // Seems like this operation can/should be made constexpr
                markConstExpr(context, ii);
                changedThisIteration = true;
            }
        }

        if( !changedThisIteration )
            return anyChanges;

        anyChanges = true;
    }
}

void maybeAddToWorkList(
    PropagateConstExprContext*  context,
    IRInst*                     gv)
{
    if( !context->onWorkList.Contains(gv) )
    {
        context->workList.add(gv);
        context->onWorkList.Add(gv);
    }
}

bool maybeMarkConstExpr(
    PropagateConstExprContext*  context,
    IRInst*                    value)
{
    if(isConstExpr(value))
        return false;

    if(!opCanBeConstExpr(value))
        return false;

    markConstExpr(context, value);

    // TODO: we should only allow function parameters to be
    // changed to be `constexpr` when we are compiling "application"
    // code, and not library code.
    // (Or eventually we'd have a rule that only non-`public` symbols
    // can have this kind of propagation applied).

    if(value->getOp() == kIROp_Param)
    {
        auto param = (IRParam*) value;
        auto block = (IRBlock*) param->parent;
        auto code = block->getParent();

        if(block == code->getFirstBlock())
        {
            // We've just changed a function parameter to
            // be `constexpr`. We need to remember that
            // fact so taht we can mark callers of this
            // function as `constexpr` themselves.

            for( auto u = code->firstUse; u; u = u->nextUse )
            {
                auto user = u->getUser();

                switch( user->getOp() )
                {
                case kIROp_Call:
                    {
                        auto inst = (IRCall*) user;
                        auto caller = as<IRGlobalValueWithCode>(inst->getParent()->getParent());
                        maybeAddToWorkList(context, caller);
                    }
                    break;

                default:
                    break;
                }
            }
        }
    }

    return true;
}

// Propagate `constexpr`-ness in a backward direction, from an instruction
// to its operands.
bool propagateConstExprBackward(
    PropagateConstExprContext*  context,
    IRGlobalValueWithCode*      code)
{
    SharedIRBuilder sharedBuilder(context->getModule());

    IRBuilder builder(sharedBuilder);
    builder.setInsertInto(code);

    bool anyChanges = false;
    for(;;)
    {
        // Note: we are walking the list of blocks and the instructions
        // in each block in reverse order, to maximize the chances that
        // we propagate multiple changes in a each pass.
        //
        // TODO: this should probably all be done with a work list instead,
        // but that requires being able to detect instructions vs. other
        // values.

        bool changedThisIteration = false;
        for( auto bb = code->getLastBlock(); bb; bb = bb->getPrevBlock() )
        {
            for( auto ii = bb->getLastInst(); ii; ii = ii->getPrevInst() )
            {
                if( isConstExpr(ii) )
                {
                    // If this instruction is `constexpr`, then its operands should be too.
                    UInt argCount = ii->getOperandCount();
                    for( UInt aa = 0; aa < argCount; ++aa )
                    {
                        auto arg = ii->getOperand(aa);
                        if(isConstExpr(arg))
                            continue;

                        if(!opCanBeConstExpr(arg))
                            continue;

                        if( maybeMarkConstExpr(context, arg) )
                        {
                            changedThisIteration = true;
                        }
                    }
                }
                else if( ii->getOp() == kIROp_Call )
                {
                    // A non-constexpr call might be calling a function with one or
                    // more constexpr parameters. We should check if we can resolve
                    // the callee for this call statically, and if so try to propagate
                    // constexpr from the parameters back to the arguments.
                    auto callInst = (IRCall*) ii;

                    UInt operandCount = callInst->getOperandCount();

                    UInt firstCallArg = 1;
                    UInt callArgCount = operandCount - firstCallArg;

                    auto callee = callInst->getOperand(0);

                    // If we are calling a generic operation, then
                    // try to follow through the `specialize` chain
                    // and find the callee.
                    //
                    // TODO: This probably shouldn't be required,
                    // since we can hopefully use the type of the
                    // callee in all cases.
                    //
                    while(auto specInst = as<IRSpecialize>(callee))
                    {
                        auto genericInst = as<IRGeneric>(specInst->getBase());
                        if(!genericInst)
                            break;

                        auto returnVal = findGenericReturnVal(genericInst);
                        if(!returnVal)
                            break;

                        callee = returnVal;
                    }

                    auto calleeFunc = as<IRFunc>(callee);
                    if(calleeFunc && isDefinition(calleeFunc))
                    {
                        // We have an IR-level function definition we are calling,
                        // and thus we can propagate `constexpr` information
                        // through its `IRParam`s.

                        auto calleeFuncType = calleeFunc->getDataType();

                        UInt callParamCount = calleeFuncType->getParamCount();
                        SLANG_RELEASE_ASSERT(callParamCount == callArgCount);

                        // If the callee has a definition, then we can read `constexpr`
                        // information off of the parameters of its first IR block.
                        if(auto calleeFirstBlock = calleeFunc->getFirstBlock())
                        {
                            UInt paramCounter = 0;
                            for(auto pp = calleeFirstBlock->getFirstParam(); pp; pp = pp->getNextParam())
                            {
                                UInt paramIndex = paramCounter++;

                                auto param = pp;
                                auto arg = callInst->getOperand(firstCallArg + paramIndex);

                                if(isConstExpr(param))
                                {
                                    if(maybeMarkConstExpr(context, arg))
                                    {
                                        changedThisIteration = true;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        // If we don't have a concrete callee function
                        // definition, then we need to extract the
                        // type of the callee instruction, and try to work
                        // with that.
                        //
                        // Note that this does not allow us to propagate
                        // `constexpr` information from the body of a callee
                        // back to call sites.
                        auto calleeType = callee->getDataType();
                        if(auto caleeFuncType = as<IRFuncType>(calleeType))
                        {
                            auto paramCount = caleeFuncType->getParamCount();
                            for( UInt pp = 0; pp < paramCount; ++pp )
                            {
                                auto paramType = caleeFuncType->getParamType(pp);
                                auto arg = callInst->getOperand(firstCallArg + pp);
                                if( isConstExpr(paramType) )
                                {
                                    if( maybeMarkConstExpr(context, arg) )
                                    {
                                        changedThisIteration = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if( bb != code->getFirstBlock() )
            {
                // A parameter in anything butr the first block is
                // conceptually a phi node, which means its operands
                // are the corresponding values from the terminating
                // branch in a predecessor block.

                UInt paramCounter = 0;
                for( auto pp = bb->getFirstParam(); pp; pp = pp->getNextParam() )
                {
                    UInt paramIndex = paramCounter++;

                    if(!isConstExpr(pp))
                        continue;

                    for(auto pred : bb->getPredecessors())
                    {
                        auto terminator = pred->getLastInst();
                        if(terminator->getOp() != kIROp_unconditionalBranch)
                            continue;

                        UInt operandIndex = paramIndex + 1;
                        SLANG_RELEASE_ASSERT(operandIndex < terminator->getOperandCount());

                        auto operand = terminator->getOperand(operandIndex);
                        if( maybeMarkConstExpr(context, operand) )
                        {
                            changedThisIteration = true;
                        }
                    }
                }
            }

        }

        if( !changedThisIteration )
            return anyChanges;

        anyChanges = true;
    }
}

// Validate use of `constexpr` within a function (in particular,
// diagnose places where a value that must be contexpr depends
// on a value that cannot be)
void validateConstExpr(
    PropagateConstExprContext*  context,
    IRGlobalValueWithCode*      code)
{
    for( auto bb = code->getFirstBlock(); bb; bb = bb->getNextBlock() )
    {
        for( auto ii = bb->getFirstInst(); ii; ii = ii->getNextInst() )
        {
            if(isConstExpr(ii))
            {
                // For an instruction that must be `constexpr`, we need
                // to ensure that its argumenst are all `constexpr`

                UInt argCount = ii->getOperandCount();
                for( UInt aa = 0; aa < argCount; ++aa )
                {
                    auto arg = ii->getOperand(aa);

                    if( !isConstExpr(arg) )
                    {
                        // Diagnose the failure.

                        context->getSink()->diagnose(ii->sourceLoc, Diagnostics::needCompileTimeConstant);

                        break;
                    }
                }
            }
        }
    }
}

void propagateConstExpr(
    IRModule*       module,
    DiagnosticSink* sink)
{
    PropagateConstExprContext context;
    context.module = module;
    context.sink = sink;
    context.sharedBuilder.init(module);
    context.builder.init(context.sharedBuilder);

    // We need to propagate information both forward and backward.
    //
    // In the forward direction we need to check if all of the operands
    // to an instruction are `constexpr` *and* if the operation is
    // one that can conceptually be "promoted" to the constexpr rate.
    //
    // In the backward direction, if an instruction has already been
    // marked as needing to be `constexpr`, then its operands had
    // better be too.
    //
    // The backward direction needs to be interprocedural, because
    // a parameter to a function might be `constexpr`, so that callers
    // of that function would need to be marked too. If backwards
    // propagation in any of the callers leads to some of their
    // parameters being marked constexpr, then we would need to
    // revisit their callers.

    // We will build an initial work list with all of the global values in it.
    
    for( auto ii : module->getGlobalInsts() )
    {
        maybeAddToWorkList(&context, ii);
    }

    // We will iterate applying propagation to one global value at a time
    // until we run out.
    while( context.workList.getCount() )
    {
        auto gv = context.workList[0];
        context.workList.fastRemoveAt(0);
        context.onWorkList.Remove(gv);

        switch( gv->getOp() )
        {
        default:
            break;

        case kIROp_Func:
        case kIROp_GlobalVar:
            {
                IRGlobalValueWithCode* code = (IRGlobalValueWithCode*) gv;

                for( ;;)
                {
                    bool anyChange = false;
                    if( propagateConstExprForward(&context, code) )
                    {
                        anyChange = true;
                    }
                    if( propagateConstExprBackward(&context, code) )
                    {
                        anyChange = true;
                    }
                    if(!anyChange)
                        break;
                }
            }
            break;
        }
    }

    // Okay, we've processed all our functions and found a steady state.
    // Now we need to try and issue diagnostics for any IR values where
    // we find that they are *required* to be `constexpr`, but *cannot*
    // be, for some reason.

    for(auto ii : module->getGlobalInsts())
    {
        switch( ii->getOp() )
        {
        default:
            break;

        case kIROp_Func:
        case kIROp_GlobalVar:
            {
                IRGlobalValueWithCode* code = (IRGlobalValueWithCode*) ii;
                validateConstExpr(&context, code);
            }
            break;
        }
    }

}

}
