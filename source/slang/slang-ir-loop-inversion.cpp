#include "slang-ir-loop-inversion.h"

#include "slang-ir-clone.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-lower-witness-lookup.h"
#include "slang-ir-reachability.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

static bool isSameBlockOrTrivialBranch(IRBlock* target, IRBlock* scrutinee)
{
    if(target == scrutinee)
        return true;
    const auto br = as<IRUnconditionalBranch>(scrutinee->getFirstOrdinaryInst());
    return br && br->getTargetBlock() == target && br->getArgCount() == 0 && !scrutinee->hasMoreThanOneUse();
};

static bool isSmallBlock(IRBlock* c)
{
    return true;
}

// Loops are suitable for inversion if:
// - The loop jumps to a conditional branch which has the break block as one of
//   its successors (or a trivial break block which we erase) and the other
//   successor is empty
// - The conditional block is "small", because we will be duplicating it
static bool isSuitableForInversion(IRLoop* loop)
{
    const auto nextBlock = loop->getTargetBlock();
    const auto breakBlock = loop->getBreakBlock();

    // The first thing a loop does must be a conditional
    const auto branch = as<IRIfElse>(nextBlock->getTerminator());
    if(!branch)
        return false;

    if(!isSmallBlock(nextBlock))
        return false;

    const auto t = branch->getTrueBlock();
    const auto f = branch->getFalseBlock();
    const auto a = branch->getAfterBlock();

    //
    // In principle we could perform this simplification in the cfg simplifier,
    // however it relies on slightly more context than is simple to insert
    // there, namely that the removed trivial branching block is branching to a
    // loop break block.
    //

    // Do we break on the 'true' side?
    if(isSameBlockOrTrivialBranch(breakBlock, t) && f == a)
    {
        if(t != breakBlock)
        {
            branch->trueBlock.set(breakBlock);
            t->removeAndDeallocate();
        }
        return true;
    }

    // ... or the false side
    if(isSameBlockOrTrivialBranch(breakBlock, f) && t == a)
    {
        if(f != breakBlock)
        {
            branch->falseBlock.set(breakBlock);
            f->removeAndDeallocate();
        }
        return true;
    }

    return false;
}

static IRParam* duplicateToParamWithDecorations(IRBuilder& builder, IRCloneEnv& cloneEnv, IRInst* i)
{
    const auto p = builder.emitParam(i->getFullType());
    for(const auto dec : i->getDecorations())
        cloneDecoration(&cloneEnv, dec, p, builder.getModule());
    return p;
}

// Given
// s: ...1 loop break=b next=c1
// c1: if x then goto b else goto d
// d: goto c1
// b: ...2
//
// Produce:
// s: ...1 goto c1
// c1: if x then goto e1 else goto l
// e1: goto b
// l: loop break=b next=d
// d: goto c2:
// c2: if x then goto e2 else goto e3
// e3: goto d
// e2: goto b
// b: ...2
//
// s is the Start block
// c1, c2 are the Condition blocks
// e1, e2, e3 are the critical Edge breakers
// l is the Loop entering block
// d is the loop boDy
// b is the Break block
static void invertLoop(IRBuilder& builder, IRLoop* loop)
{
    IRBuilderInsertLocScope builderScope(&builder);
    const auto s = as<IRBlock>(loop->getParent());
    auto domTree = computeDominatorTree(s->getParent());
    SLANG_ASSERT(s);
    const auto c1 = loop->getTargetBlock();
    const auto c1Terminator = as<IRConditionalBranch>(c1->getTerminator());
    SLANG_ASSERT(c1Terminator);
    const auto b = loop->getBreakBlock();
    auto& c1dUse = c1Terminator->getTrueBlock() == b ? c1Terminator->falseBlock : c1Terminator->trueBlock;
    auto& c1bUse = c1Terminator->getTrueBlock() == b ? c1Terminator->trueBlock : c1Terminator->falseBlock;
    const auto d = as<IRBlock>(c1dUse.get());
    SLANG_ASSERT(d);

    IRCloneEnv cloneEnv;
    cloneEnv.squashChildrenMapping = true;

    // Since we are duplicating the loop break condition block (c1) we must
    // introduce phi values for anything in it upon which the rest of the
    // program (b onwards) uses. Lift the values fron c1 used in b (and
    // onwards) to parameters. To avoid a critical edge, pass these via a new
    // block, e1.
    builder.setInsertInto(b);
    List<IRInst*> c1Params;
    for(auto i : IRInstList<IRInst>(c1->getFirstInst(), c1->getLastInst()))
    {
        IRParam* p = nullptr;
        traverseUses(i, [&](IRUse* u){
            auto userBlock = u->getUser()->getParent();
            if(domTree->dominates(b, userBlock))
            {
                // A new parameter to replace this 'i'
                if(!p)
                    p = duplicateToParamWithDecorations(builder, cloneEnv, i);
                u->set(p);
            }
        });
        if(p)
            c1Params.add(i);
    }
    auto e1 = builder.emitBlock();
    e1->insertAfter(c1);
    builder.emitBranch(b, c1Params.getCount(), c1Params.getBuffer());
    c1bUse.set(e1);
    // Similarly, we have to replace any existing 'break's to break via e1
    traverseUses(b, [&](IRUse* u){
        auto userBlock = u->getUser()->getParent();
        // Restrict this to just those blocks within this loop
        if(userBlock != e1 && domTree->dominates(s, userBlock) && !domTree->dominates(b, userBlock))
            u->set(e1);
    });
    // We now have
    // s: ...1 loop break=b next=c1
    // c1: if x then goto e1 else goto d
    // e1: goto b
    // d: goto c1
    // b: ...2

    // Duplicate c1 into c2, and using the same cloneEnv, duplicate e1 into e2
    builder.setInsertInto(builder.getFunc());
    const auto c2 = as<IRBlock>(cloneInst(&cloneEnv, &builder, c1));
    c2->insertBefore(c1);
    const auto e2 = as<IRBlock>(cloneInst(&cloneEnv, &builder, e1));
    e2->insertAfter(c2);
    const auto c2Terminator = as<IRConditionalBranch>(c2->getTerminator());
    auto& c2eUse = c2Terminator->getTrueBlock() == e1 ? c2Terminator->trueBlock : c2Terminator->falseBlock;
    c2eUse.set(e2);
    builder.setInsertAfter(c2Terminator);
    const auto newC2Terminator = builder.emitIfElse(c2Terminator->getCondition(), c2Terminator->getTrueBlock(), c2Terminator->getFalseBlock(), b);
    c2Terminator->removeAndDeallocate();
    // We now have
    // s: ...1 loop break=b next=c1
    // c2: if x then goto e2 else goto d
    // e2: goto b
    // c1: if x then goto e1 else goto d
    // e1: goto b
    // d: goto c1
    // b: ...2

    // move the loop instruction to its own block, l
    const auto l = builder.emitBlock();
    l->insertAfter(e2);
    loop->insertAtEnd(l);
    // We now have
    // s: ...1 no-termiator
    // c2: if x then goto e2 else goto d
    // e2: goto b
    // l: loop break=b next=c1
    // c1: if x then goto e1 else goto d
    // e1: goto b
    // d: goto c1
    // b: ...2

    // add a new terminator to s. A jump to c2, our outer conditional. retain
    // any parameters the loop instruction passed to c1
    builder.setInsertInto(s);
    List<IRInst*> as;
    for(UInt i = 0; i < loop->getArgCount(); ++i)
        as.add(loop->getArg(i));
    builder.emitBranch(c2, as.getCount(), as.getBuffer());
    // We now have
    // s: ...1, goto c2
    // c2: if x then goto e2 else goto d
    // e2: goto b
    // l: loop break=b next=c1
    // c1: if x then goto e1 else goto d
    // e1: goto b
    // d: goto c1
    // b: ...2

    // modify c2 to jump to the new loop
    auto& c2dUse = newC2Terminator->getTrueBlock() == e2 ? newC2Terminator->falseBlock : newC2Terminator->trueBlock;
    c2dUse.set(l);
    // We now have
    // s: ...1, goto c2
    // c2: if x then goto e2 else goto l
    // e2: goto b
    // l: loop break=b next=c1
    // c1: if x then goto e1 else goto d
    // e1: goto b
    // d: goto c1
    // b: ...2

    //
    // Now we can modify the loop to jump to the block after the first
    // conditional, d, as we know that it won't break out of the loop on the
    // first iteration
    //
    // Beyond just retargeting the loop instruction, we need to make sure any
    // parameters the loop instruction is passing to c1 are instead passed to
    // 'd', and because we've added parameters to 'd' we need to forward them
    // from c1 also which we will accomplish using a new block, e3,
    loop->block.set(d);
    loop->breakBlock.set(e1);
    SLANG_ASSERT(d->getFirstParam() == nullptr);
    c1->insertBefore(b);
    e1->insertAfter(c1);
    List<IRInst*> ps;
    for(const auto p : c1->getParams())
        ps.add(p);
    builder.setInsertInto(d);
    for(const auto p : ps)
    {
        const auto q = duplicateToParamWithDecorations(builder, cloneEnv, p);
        // Replace all uses, except for those in c1 and e1
        List<IRUse*> uses;
        traverseUses(p, [&](IRUse* u){if(u->user->getParent() != c1 && u->user->getParent() != e1) uses.add(u);});
        for(auto u : uses)
            u->set(q);
    }
    const auto e3 = builder.emitBlock();
    e3->insertAfter(c1);
    builder.emitBranch(d, ps.getCount(), ps.getBuffer());
    c1dUse.set(e3);
    // We now have the desired output
    // s: ...1, goto c2
    // c2: if x then goto e2 else goto l
    // e2: goto b
    // l: loop break=e1 next=d
    // d: goto c1
    // c1: if x then goto e1 else goto e3
    // e3: goto d
    // e1: goto b
    // b: ...2
}

bool invertLoops(IRModule* module)
{
    IRBuilder builder(module);
    List<IRLoop*> toInvert;
    overAllBlocks(module, [&](auto b){
        if(auto loop = as<IRLoop>(b->getTerminator()))
        {
            if(isSuitableForInversion(loop))
                toInvert.add(loop);
        }
    });
    for(const auto loop : toInvert)
        invertLoop(builder, loop);
    return toInvert.getCount() > 0;
}

} // namespace Slang
