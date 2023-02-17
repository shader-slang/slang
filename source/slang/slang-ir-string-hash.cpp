// slang-ir-string-hash.cpp
#include "slang-ir-string-hash.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang {

static void _findGetStringHashRec(IRInst* inst, List<IRGetStringHash*>& outInsts)
{
    for (IRInst* child = inst->getFirstDecorationOrChild(); child; child = child->getNextInst())
    {
        if (IRGetStringHash* getInst = as<IRGetStringHash>(child))
        {
            outInsts.add(getInst);
        }
        _findGetStringHashRec(child, outInsts);
    }
}

void findGetStringHashInsts(IRModule* module, List<IRGetStringHash*>& outInsts)
{
    _findGetStringHashRec(module->getModuleInst(), outInsts);
}

void findGlobalHashedStringLiterals(IRModule* module, StringSlicePool& pool)
{
    IRModuleInst* moduleInst = module->getModuleInst();

    for(IRInst* child : moduleInst->getChildren())
    {
        if (IRGlobalHashedStringLiterals* hashedStringLits = as<IRGlobalHashedStringLiterals>(child))
        {
            const Index count = hashedStringLits->getOperandCount();
            for (Index i = 0; i < count; ++i)
            {
                IRStringLit* stringLit = as<IRStringLit>(hashedStringLits->getOperand(i));
                pool.add(stringLit->getStringSlice());
            }
        }
    }
}

void addGlobalHashedStringLiterals(const StringSlicePool& pool, IRModule* module)
{
    auto slices = pool.getAdded();
    if (slices.getCount() == 0)
    {
        return;
    }

    IRBuilder builder(module);
    
    // We need to add a global instruction that references all of these string literals
    builder.setInsertInto(module->getModuleInst());

    const Index slicesCount = slices.getCount();

    // Note: This pass is using the extremely low-level `_allocateInst` operation on `IRModule`
    // as an optimization. By allocating the instruction here in an "empty" state and then filling
    // its operands in in place, we can avoid allocating space for a temporary `List<IRInst*>` to
    // hold the IR string values created in the loop below.
    //
    // TODO: We should probably either eliminate this micro-optimization and just use a `List<IRInst*>`
    // here, *or* we should devise a more first-class system for doing in-place instruction creation
    // like that that can be compatible with desirable features like automatic deduplication.
    //
    IRInst* globalHashedInst = module->_allocateInst(kIROp_GlobalHashedStringLiterals, int(slicesCount));
    builder.addInst(globalHashedInst);

    auto operands = globalHashedInst->getOperands();

    for (Index i = 0; i < slicesCount; ++i)
    {
        IRStringLit* stringLit = builder.getStringValue(slices[i]);
        operands[i].init(globalHashedInst, stringLit);
    }

    // Mark to keep alive
    builder.addKeepAliveDecoration(globalHashedInst);
}

Result checkGetStringHashInsts(IRModule* module, DiagnosticSink* sink)
{
    // Check all getStringHash are all on string literals
    List<IRGetStringHash*> insts;
    findGetStringHashInsts(module, insts);

    for (auto inst : insts)
    {
        if (inst->getStringLit() == nullptr)
        {
            if (sink)
            {
                sink->diagnose(inst, Diagnostics::getStringHashMustBeOnStringLiteral);
            }

            // Doesn't access a string literal
            return SLANG_FAIL;
        }
    }
    
    return SLANG_OK;
}

}
