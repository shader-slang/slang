#include "slang-ir-fuse-satcoop.h"

#include "slang-ir-insts.h"
#include "slang-ir-specialize-function-call.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir.h"

namespace Slang
{

//
// Some helpers
//

// Run an operation over every block in a module
template<typename F>
static void overAllBlocks(IRModule* module, F f)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto func = as<IRGlobalValueWithCode>(globalInst))
        {
            for (auto block : func->getBlocks())
            {
                f(block);
            }
        }
    }
}

// Given a specialize call, replace the first parameters with the given ones
static IRInst* respecialize(IRBuilder& builder, IRSpecialize* spec, List<IRInst*> newArgs)
{
    SLANG_ASSERT(Index(spec->getArgCount()) >= newArgs.getCount());

    const auto generic = spec->getBase();

    // Copy over any witness tables
    for(UInt i = newArgs.getCount(); i < spec->getArgCount(); ++i)
        newArgs.add(spec->getArg(i));
    return builder.emitSpecializeInst(builder.getTypeKind(), generic, newArgs);
}

static bool uses(IRInst* used, IRInst* user)
{
    for(auto use = used->firstUse; use; use = use->nextUse)
    {
        if(use->getUser() == user)
            return true;
    }
    return false;
};

// given: `f; x; g`
// reorder instructions such that f and g are adjacent, in the form:
// `p; f; g; q`, 
//
// p is the set of instructions upon which g depends and q is the
// set of instructions which depend on f. If these sets are not disjoint then
// we can't float f and g together. Instructions not used by g and which don't
// use f can go in either p or q.
//
// Returns g on success
static IRInst* floatTogether(IRInst* f, IRInst* g)
{
    List<IRInst*> ps, qs;

    auto usesF = [&](IRInst* i){
        if(uses(f, i))
            return true;
        for(auto q : qs)
            if(uses(q, i))
                return true;
        return false;
    };
    auto usedByG = [&](IRInst* i){
        if(uses(i, g))
            return true;
        for(auto p : ps)
            if(uses(i, p))
                return true;
        return false;
    };

    // Scan backwards to find which instructions g depends on, known as p
    auto i = g->prev;
    while(i != f)
    {
        SLANG_ASSERT(i);

        // If any instruction in x has side effects, we can't reorder things
        if(i->mightHaveSideEffects())
            return nullptr;
            
        if(usedByG(i))
            ps.add(i);
        i = i->prev;
    }

    // Scan forwards to compute instructions which depend on f, the instructions in q
    i = f->next;
    while(i != g)
    {
        if(usesF(i))
        {
            // If this happens then ps and qs are not disjoint, and we will not
            // be able to float f and g together
            if(ps.contains(i))
                return nullptr;
            qs.add(i);
        }

        i = i->next;
    }

    // Now we can safely reorder things by moving p;f;g before everything else
    // Remember, we constructed ps in reverse, so we must insert these
    // backwards too
    for(Index j = ps.getCount()-1; j >= 0; --j)
    {
        auto p = ps[j];
        p->removeFromParent();
        p->insertBefore(f);
    }
    g->removeFromParent();
    g->insertAfter(f);
    return g;
}

// bifanout(f, g)(x, (a, b)) = (f(x, a), g(x, b))
//
// Make a function `bifanout` which applies one function after another to the
// same input, while also distributing a pair of distinct inputs amongst the
// functions
//
// The outputs are returned in a 2-tuple
static IRFunc* makeBiFanout(IRBuilder& builder, IRFunc* f, IRFunc* g)
{
    SLANG_ASSERT(f->getParamCount() == 2);
    SLANG_ASSERT(g->getParamCount() == f->getParamCount());
    SLANG_ASSERT(f->getParamType(0) == g->getParamType(0));
    IRBuilderInsertLocScope insertLocScope(&builder);

    // Create
    // func myFunc(s : S, u : (U1,U2)) -> (R1, R2)
    // {
    //     let fRes = f(s, u.fst);
    //     let gRes = g(s, u.snd);
    //     return (fRes, gRes);
    // }

    // The return type is the tuple of f and g's return types
    auto resType = builder.getTupleType(f->getResultType(), g->getResultType());
    auto sharedInputType = f->getParamType(0);
    // likewise, the distinct input type is the product of this input to the two functions
    auto unsharedInputType = builder.getTupleType(f->getParamType(1), g->getParamType(1));

    // Set up our function
    // func myFunc(s : S, u : (U1,U2)) -> (R1, R2)
    auto func = builder.createFunc();
    builder.addDecoration(func, kIROp_ForceInlineDecoration);
    builder.setDataType(func, builder.getFuncType({sharedInputType, unsharedInputType}, resType));
    builder.setInsertInto(func);
    auto b = builder.emitBlock();
    builder.setInsertInto(b);
    auto s = builder.emitParam(sharedInputType);
    auto u = builder.emitParam(unsharedInputType);

    //     let fRes = f(s, u.fst);
    auto fRes = builder.emitCallInst(f->getResultType(), f, {s, builder.emitGetTupleElement(f->getParamType(1), u, 0)});
    //     let gRes = g(s, u.snd);
    auto gRes = builder.emitCallInst(g->getResultType(), g, {s, builder.emitGetTupleElement(g->getParamType(1), u, 1)});
    //     return (fRes, gRes);
    builder.emitReturn(builder.emitMakeTuple(fRes, gRes));
    return func;
}

// fanout(f, g)(x, a) = (f(x, a), g(x, a))
// or: fanout f g = f &&& g
// 
// (We distinguish functions taking two arguments from functions taking one
// 2-tuple argument, hence the expansion of (x,a) above)
//
// This is very similar to above, except that f and g have the same second
// parameter.
static IRFunc* makeFanout(
    IRBuilder& builder,
    IRFunc* f, 
    IRFunc* g)
{
    SLANG_ASSERT(f->getParamCount() == 2);
    SLANG_ASSERT(g->getParamCount() == f->getParamCount());
    for(UInt i = 0; i < f->getParamCount(); ++i)
        SLANG_ASSERT(f->getParamType(i) == g->getParamType(i));
    IRBuilderInsertLocScope insertLocScope(&builder);

    // Create
    // func myFunc(s : S, u : U) -> (R1, R2)
    // {
    //     let fRes = f(s, u);
    //     let gRes = g(s, u);
    //     return (fRes, gRes);
    // }

    // We still return the results of both f and g in a tuple
    auto resType = builder.getTupleType(f->getResultType(), g->getResultType());
    auto sharedInputType = f->getParamType(0);
    // Differing from above, it's the same type for f and g
    auto unsharedInputType = f->getParamType(1);

    // Set up our function
    // func myFunc(s : S, u : U) -> (R1, R2)
    auto func = builder.createFunc();
    builder.addDecoration(func, kIROp_ForceInlineDecoration);
    builder.setDataType(func, builder.getFuncType({sharedInputType, unsharedInputType}, resType));
    builder.setInsertInto(func);
    auto b = builder.emitBlock();
    builder.setInsertInto(b);
    auto s = builder.emitParam(sharedInputType);
    auto u = builder.emitParam(unsharedInputType);

    //     let fRes = f(s, u);
    auto fRes = builder.emitCallInst(f->getResultType(), f, {s, u});
    //     let gRes = g(s, u);
    auto gRes = builder.emitCallInst(g->getResultType(), g, {s, u});
    //     return (fRes, gRes);
    builder.emitReturn(builder.emitMakeTuple(fRes, gRes));
    return func;
}

struct SatCoopCall
{
    // The definition in hlsl.slang
    IRGeneric* generic;

    // The specialization of that call 
    IRSpecialize* specialize;

    // Called 'A' in the definition
    IRType* sharedInputType;
    // Called 'B' in the definition
    IRType* distinctInputType;
    // Called 'C' in the definition
    IRType* retType;

    // The function arguments to the call
    IRFunc* cooperate;
    IRFunc* fallback;

    // The values to pass to these functions
    IRInst* sharedInput;
    IRInst* distinctInput;
};

static SatCoopCall getSatCoopCall(IRCall* f)
{
    SatCoopCall ret;
    ret.specialize = as<IRSpecialize>(f->getCallee());

    // Since this is a call to saturated_cooperation, it must have at least
    // three specialization arguments for the type parameters A, B, C. We allow
    // more here for any dictionaries or witnesses. 
    SLANG_ASSERT(ret.specialize && ret.specialize->getArgCount() >= 3);
    ret.generic = as<IRGeneric>(ret.specialize->getBase());
    SLANG_ASSERT(ret.generic);
    ret.sharedInputType = as<IRType>(ret.specialize->getArg(0));
    ret.distinctInputType = as<IRType>(ret.specialize->getArg(1));
    ret.retType = as<IRType>(ret.specialize->getArg(2));
    SLANG_ASSERT(ret.sharedInputType);
    SLANG_ASSERT(ret.distinctInputType);
    SLANG_ASSERT(ret.retType);
    
    SLANG_ASSERT(f->getArgCount() == 4);
    ret.cooperate = as<IRFunc>(f->getArg(0));
    ret.fallback = as<IRFunc>(f->getArg(1));
    SLANG_ASSERT(ret.cooperate);
    SLANG_ASSERT(ret.fallback);

    ret.sharedInput = f->getArg(2);
    ret.distinctInput = f->getArg(3);
    SLANG_ASSERT(ret.sharedInput->getDataType() == ret.sharedInputType);
    SLANG_ASSERT(ret.distinctInput->getDataType() == ret.distinctInputType);
    return ret;
}

// transform:
//     a = sat_coop(c1, f1, s, u1); // f
//     p;
//     q;
//     b = sat_coop(c2, f2, s, u2); // g
// to:
//     p;
//     (a,b) = sat_coop(c1 &&& c2, f1 &&& f2, s, (u1, u2));
//     q;
//
// Removes the first two calls, and returns the second one if creation was
// successful. 
// 
// This can fail if:
//
// p has side effects which c1 or f1 may depend on
// q has side effects which c2 or f2 may depend on
// p depends on a
// the second call to sat_coop depends on a
// the second call to sat_coop depends on q
static IRCall* tryFuseCalls(IRBuilder& builder, IRCall* f, IRCall* g)
{
    IRBuilderInsertLocScope insertLocScope(&builder);

    SatCoopCall callF = getSatCoopCall(f);
    SatCoopCall callG = getSatCoopCall(g);
    // If these aren't referencing the same generic, then something has gone
    // wrong in our assumptions.
    SLANG_ASSERT(callF.generic == callG.generic);

    // If these don't use the same shared input then we can't fuse them
    if(callF.sharedInput != callG.sharedInput)
        return nullptr;

    // If g uses the result of f, we can't fuse them with this logic (we could
    // however with a replacement for 'fanout') 
    if(uses(f, g))
        return nullptr;

    // If there is no safe way to float these together, then fail
    const auto q = floatTogether(f, g);
    if(!q)
        return nullptr;
    builder.setInsertBefore(q);

    // As a slight neatening, we'll avoid wrapping and upwrapping a tuple (u,u)
    // if both f and g use the same distinct input..
    bool usesSameDistinctInput = callF.distinctInput == callG.distinctInput;
    SLANG_ASSERT(!usesSameDistinctInput || callF.distinctInputType == callG.distinctInputType);

    // Generate a new specialization of our saturated_cooperation function,
    // reflecting the new input and output types. 
    // Make sure that any additional specialization arguments are identical,
    // which according to the definition of saturated_cooperation when this
    // fuse pass was written must be true.
    for(UInt i = 3; i < callF.specialize->getArgCount(); ++i)
        SLANG_ASSERT(callF.specialize->getArg(i) == callG.specialize->getArg(i));
    const auto newRetType = builder.getTupleType(callF.retType, callG.retType);
    const auto distinctInputType = usesSameDistinctInput 
        ? callF.distinctInputType 
        : builder.getTupleType(callF.distinctInputType, callG.distinctInputType);
    const auto newSpec = respecialize(builder, callF.specialize, {
        callF.sharedInputType,
        distinctInputType,
        newRetType
    });

    // Make our new functions, and joined inputs
    const auto fanout = usesSameDistinctInput ? makeFanout : makeBiFanout;
    const auto newCooperate = fanout(builder, callF.cooperate, callG.cooperate);
    const auto newFallback = fanout(builder, callF.fallback, callG.fallback);
    const auto newSharedInput = callF.sharedInput;
    const auto newDistinctInput = usesSameDistinctInput 
        ? callF.distinctInput 
        : builder.emitMakeTuple(callF.distinctInput, callG.distinctInput);

    // Call it and extract the results from f and g
    const auto res = builder.emitCallInst(newRetType, newSpec, {newCooperate, newFallback, newSharedInput, newDistinctInput});
    const auto resF = builder.emitGetTupleElement(callF.retType, res, 0);
    const auto resG = builder.emitGetTupleElement(callG.retType, res, 1);
    f->replaceUsesWith(resF);
    g->replaceUsesWith(resG);
    f->removeAndDeallocate();
    g->removeAndDeallocate();

    return res;
}

//
// Identify calls which we can fuse
//
bool isSatCoopCall(IRCall* call)
{
    // saturated_cooperation is a generic function, so look for specializations thereof
    auto spec = as<IRSpecialize>(call->getCallee());
    if(!spec)
        return false;
    auto func = findSpecializeReturnVal(spec);
    if(!func)
        return false;

    // TODO: Is there nothing better for finding a specific declaration in
    // core.slang?
    auto h = func->findDecoration<IRNameHintDecoration>();
    return h->getName() == "saturated_cooperation";
}

static void fuseCallsInBlock(IRBuilder& builder, IRBlock* block)
{
    // Walk over the instructions in this block
    // If we see a call to sat_coop then remember where it is and keep
    // walking, if we reach another call without first encountering any
    // instructions with which our first call can't be safely reordered
    // then we remove the first call and replace the second with a fused
    // call.
    IRCall* lastCall = nullptr;
    for(auto inst = block->getFirstInst(); inst != block->getTerminator(); inst = inst->getNextInst())
    {
        if (auto call = as<IRCall>(inst))
        {
            if(isSatCoopCall(call))
            {
                if(lastCall)
                {
                    auto fused = tryFuseCalls(builder, lastCall, call);
                    if(fused)
                    {
                        inst = fused;
                        lastCall = fused;
                    }
                    else
                    {
                        lastCall = call;
                    }
                }
                else 
                {
                    lastCall = call;
                }
            }
        }
    }
}

void fuseCallsToSaturatedCooperation(IRModule* module)
{
    IRBuilder builder(module);
    overAllBlocks(module, [&](auto b){fuseCallsInBlock(builder, b);});
}

} // namespace Slang
