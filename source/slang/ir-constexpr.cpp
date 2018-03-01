// ir-constexpr.cpp
#include "ir-constexpr.h"

#include "ir.h"
#include "ir-insts.h"

namespace Slang {

struct PropagateConstExprContext
{
    IRModule* module;
    IRModule* getModule() { return module; }

    DiagnosticSink* sink;

    SharedIRBuilder sharedBuilder;
    IRBuilder builder;

    List<IRGlobalValue*> workList;
    HashSet<IRGlobalValue*> onWorkList;

    IRBuilder* getBuilder() { return &builder; }

    Session* getSession() { return sharedBuilder.session; }

    DiagnosticSink* getSink() { return sink; }
};

bool isConstExpr(Type* type)
{
    if( auto rateQualifiedType = type->As<RateQualifiedType>() )
    {
        auto rate = rateQualifiedType->rate;
        if(auto constExprRate = rate->As<ConstExprRate>())
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
    switch(value->op)
    {
    case kIROp_IntLit:
    case kIROp_FloatLit:
    case kIROp_boolConst:
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
    case kIROp_boolConst:
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_Mod:
    case kIROp_Neg:
    case kIROp_Construct:
    case kIROp_makeVector:
    case kIROp_makeArray:
    case kIROp_makeMatrix:
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

    return opCanBeConstExpr(value->op);
}

void markConstExpr(
    PropagateConstExprContext*  context,
    IRInst*                    value)
{
    Slang::markConstExpr(context->getSession(), value);
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
    IRGlobalValue*              gv)
{
    if( !context->onWorkList.Contains(gv) )
    {
        context->workList.Add(gv);
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

    if(value->op == kIROp_Param)
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

                switch( user->op )
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
    SharedIRBuilder sharedBuilder;
    sharedBuilder.module = context->getModule();
    sharedBuilder.session = sharedBuilder.module->session;

    IRBuilder builder;
    builder.sharedBuilder = &sharedBuilder;
    builder.curFunc = code;
    builder.curBlock = nullptr;

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
                else if( ii->op == kIROp_Call )
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
                    while( callee->op == kIROp_specialize )
                    {
                        callee = ((IRSpecialize*) callee)->getOperand(0);
                    }
                    if( callee->op == kIROp_Func )
                    {
                        auto calleeFunc = (IRFunc*) callee;
                        auto calleeFuncType = calleeFunc->getType();

                        UInt callParamCount = calleeFuncType->getParamCount();
                        SLANG_RELEASE_ASSERT(callParamCount == callArgCount);

                        // If the callee has a definition, then we can read `constexpr`
                        // information off of the parameters of its first IR block.
                        if( auto calleeFirstBlock = calleeFunc->getFirstBlock() )
                        {
                            UInt paramCounter = 0;
                            for( auto pp = calleeFirstBlock->getFirstParam(); pp; pp = pp->getNextParam() )
                            {
                                UInt paramIndex = paramCounter++;

                                auto param = pp;
                                auto arg = callInst->getOperand(firstCallArg + paramIndex);

                                if( isConstExpr(param) )
                                {
                                    if( maybeMarkConstExpr(context, arg) )
                                    {
                                        changedThisIteration = true;
                                    }
                                }
                            }
                        }
                        else
                        {
                            // If we don't have the definition/body for the callee,
                            // then we have to glean `constexpr` information from its
                            // type instead.
                            auto calleeType = calleeFunc->getType();
                            auto paramCount = calleeType->getParamCount();
                            for( UInt pp = 0; pp < paramCount; ++pp )
                            {
                                auto paramType = calleeType->getParamType(pp);
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
                        if(terminator->op != kIROp_unconditionalBranch)
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
    auto session = module->session;

    PropagateConstExprContext context;
    context.module = module;
    context.sink = sink;
    context.sharedBuilder.module = module;
    context.sharedBuilder.session = session;
    context.builder.sharedBuilder = &context.sharedBuilder;
    context.builder.curFunc = nullptr;
    context.builder.curBlock = nullptr;



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
        auto gv = as<IRGlobalValue>(ii);
        if (!gv)
            continue;
        maybeAddToWorkList(&context, gv);
    }

    // We will iterate applying propagation to one global value at a time
    // until we run out.
    while( context.workList.Count() )
    {
        auto gv = context.workList[0];
        context.workList.FastRemoveAt(0);
        context.onWorkList.Remove(gv);

        switch( gv->op )
        {
        default:
            break;

        case kIROp_Func:
        case kIROp_global_var:
        case kIROp_global_constant:
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
        switch( ii->op )
        {
        default:
            break;

        case kIROp_Func:
        case kIROp_global_var:
        case kIROp_global_constant:
            {
                IRGlobalValueWithCode* code = (IRGlobalValueWithCode*) ii;
                validateConstExpr(&context, code);
            }
            break;
        }
    }

}

}
