// ir-clone.cpp
#include "ir-clone.h"

#include "ir.h"
#include "ir-insts.h"

namespace Slang
{

IRInst* lookUp(IRCloneEnv* env, IRInst* oldVal)
{
    for( auto ee = env; ee; ee = ee->parent )
    {
        IRInst* newVal = nullptr;
        if(ee->mapOldValToNew.TryGetValue(oldVal, newVal))
            return newVal;
    }
    return nullptr;
}

IRInst* findCloneForOperand(
    IRCloneEnv*     env,
    IRInst*         oldOperand)
{
    if(!oldOperand) return nullptr;

    // If there is a registered replacement for
    // the existing operand, then use it.
    //
    if( IRInst* newVal = lookUp(env, oldOperand) )
        return newVal;

    // Otherwise, we assume that the caller wants
    // to default to using existing values wherever
    // an explicit replacement hasn't been registered.
    //
    // This is, notably, the right default whenever
    // `oldOperand` is a global value or constant
    // and our cloned code will sit in the same
    // module as the original.
    //
    // TODO: We could make this a customization point
    // down the road, if we ever had a case where
    // we want to clone things with a different policy.
    //
    return oldOperand;
}

IRInst* cloneInstAndOperands(
    IRCloneEnv*     env,
    IRBuilder*      builder,
    IRInst*         oldInst)
{
    SLANG_ASSERT(env);
    SLANG_ASSERT(builder);
    SLANG_ASSERT(oldInst);

    // This logic will not handle any instructions
    // with special-case data attached, but that only
    // applies to `IRConstant`s at this point, and those
    // should only appear at the global scope rather than
    // in function bodies.
    //
    // TODO: It would be easy enough to extend this logic
    // to handle constants gracefully, if it ever comes up.
    //
    SLANG_ASSERT(!as<IRConstant>(oldInst));

    // We start by mapping the type of the orignal instruction
    // to its replacement value, if any.
    //
    auto oldType = oldInst->getFullType();
    auto newType = (IRType*) findCloneForOperand(env, oldType);

    // Next we will create an empty shell of the instruction,
    // with space for the operands, but no actual operand
    // values attached.
    //
    UInt operandCount = oldInst->getOperandCount();
    auto newInst = builder->emitIntrinsicInst(
        newType,
        oldInst->op,
        operandCount,
        nullptr);

    // Finally we will iterate over the operands of `oldInst`
    // to find their replacements and install them as
    // the operands of `newInst`.
    //
    for(UInt ii = 0; ii < operandCount; ++ii)
    {
        auto oldOperand = oldInst->getOperand(ii);
        auto newOperand = findCloneForOperand(env, oldOperand);

        newInst->getOperands()[ii].init(newInst, newOperand);
    }

    return newInst;
}

// The complexity of the second phase of cloning (the
// one that deals with decorations and children) comes
// from the fact that it needs to sequence the two phases
// of cloning for any child instructions. We will do this
// by performing the first phase of cloning, and building
// up a list of children that require the second phase of processing.
// Each entry in that list will be a pair of an old instruction
// and its new clone.
//
struct IRCloningOldNewPair
{
    IRInst* oldInst;
    IRInst* newInst;
};

// We will use an internal variant of `cloneInstDecorationsAndChildren`
// that modifies the provided `env` as it goes as the main
// workhorse, since we need to make sure that instructions in
// earlier blocks are visible to those in other, later, blocks
// when cloning a function, so that strict scoping along the
// lines of the nesting of instructions isn't sufficient.
//
static void _cloneInstDecorationsAndChildren(
    IRCloneEnv*         env,
    SharedIRBuilder*    sharedBuilder,
    IRInst*             oldInst,
    IRInst*             newInst)
{
    SLANG_ASSERT(env);
    SLANG_ASSERT(sharedBuilder);
    SLANG_ASSERT(oldInst);
    SLANG_ASSERT(newInst);

    // We will set up an IR builder that inserts
    // into the new parent instruction.
    //
    IRBuilder builderStorage;
    auto builder = &builderStorage;
    builder->sharedBuilder = sharedBuilder;
    builder->setInsertInto(newInst);

    // When applying the first phase of cloning to
    // children, we will keep track of those that
    // require the second phase.
    //
    List<IRCloningOldNewPair> pairs;

    for( auto oldChild : oldInst->getDecorationsAndChildren() )
    {
        // As a very subtle special case, if one of the children
        // of our `oldInst` already has a registered replacement,
        // then we don't want to clone it (not least because
        // the `Dictionary::Add` method would give us an error
        // when we try to insert a new value for the same key).
        //
        // This arises for entries in `mapOldValToNew` that were
        // seeded before cloning begain (e.g., function
        // parameters that are to be replaced).
        //
        if(lookUp(env, oldChild))
            continue;

        // Now we can perform the first phase of cloning
        // on the child, and register it in our map from
        // old to new values.
        //
        auto newChild = cloneInstAndOperands(env, builder, oldChild);
        env->mapOldValToNew.Add(oldChild, newChild);

        // If and only if the old child had decorations
        // or children, we will register it into our
        // list for processing in the second phase.
        //
        if( oldChild->getFirstDecorationOrChild() )
        {
            IRCloningOldNewPair pair;
            pair.oldInst = oldChild;
            pair.newInst = newChild;
            pairs.Add(pair);
        }
    }

    // Once we have done first-phase processing for
    // all child instructions, we scan through those
    // in the list that required second-phase processing,
    // and clone their decorations and/or children recursively.
    //
    for( auto pair : pairs )
    {
        auto oldChild = pair.oldInst;
        auto newChild = pair.newInst;

        _cloneInstDecorationsAndChildren(env, sharedBuilder, oldChild, newChild);
    }
}

// The public version of `cloneInstDecorationsAndChildren` is then
// just a wrapper over the internal one that sets up a temporary
// environment to use for the cloning process, so that we do
// not leave any lasting changes in the user-provided `env`.
//
void cloneInstDecorationsAndChildren(
    IRCloneEnv*         env,
    SharedIRBuilder*    sharedBuilder,
    IRInst*             oldInst,
    IRInst*             newInst)
{
    SLANG_ASSERT(sharedBuilder);
    SLANG_ASSERT(oldInst);
    SLANG_ASSERT(newInst);

    IRCloneEnv subEnvStorage;
    auto subEnv = &subEnvStorage;
    subEnv->parent = env;

    _cloneInstDecorationsAndChildren(subEnv, sharedBuilder, oldInst, newInst);
}

// The convenience function `cloneInst` just sequences the
// operations that have already been defined.
//
IRInst* cloneInst(
    IRCloneEnv*     env,
    IRBuilder*      builder,
    IRInst*         oldInst)
{
    SLANG_ASSERT(env);
    SLANG_ASSERT(builder);
    SLANG_ASSERT(oldInst);

    auto newInst = cloneInstAndOperands(
        env, builder, oldInst);

    env->mapOldValToNew.Add(oldInst, newInst);

    cloneInstDecorationsAndChildren(
        env, builder->sharedBuilder, oldInst, newInst);

    return newInst;
}

bool IRSimpleSpecializationKey::operator==(IRSimpleSpecializationKey const& other) const
{
    auto valCount = vals.Count();
    if(valCount != other.vals.Count()) return false;
    for( UInt ii = 0; ii < valCount; ++ii )
    {
        if(vals[ii] != other.vals[ii]) return false;
    }
    return true;
}

int IRSimpleSpecializationKey::GetHashCode() const
{
    auto valCount = vals.Count();
    int hash = Slang::GetHashCode(valCount);
    for( UInt ii = 0; ii < valCount; ++ii )
    {
        hash = combineHash(hash, Slang::GetHashCode(vals[ii]));
    }
    return hash;
}


} // namespace Slang
