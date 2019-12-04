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

void replaceGetStringHash(IRModule* module, SharedIRBuilder& sharedBuilder, StringSlicePool& ioPool)
{
    IRBuilder builder;
    builder.sharedBuilder = &sharedBuilder;

    builder.setInsertInto(module->getModuleInst());

    List<IRGetStringHash*> insts;
    _findGetStringHashRec(module->getModuleInst(), insts);

    // Then we want to add the GlobalHashedString instruction in the root
    for (auto inst : insts)
    {
        IRStringLit* stringLit = inst->getStringLit();
        ioPool.add(stringLit->getStringSlice());
        
        // Okay work out what the hash is
        const int hash = GetHashCode(stringLit->getStringSlice());

        IRInst* intLit = builder.getIntValue(builder.getIntType(), int32_t(hash));

        // Okay we want to replace all uses with the literal
        inst->replaceUsesWith(intLit);
        inst->removeAndDeallocate();
    }
}

void replaceGetStringHashWithGlobal(IRModule* module, SharedIRBuilder& sharedBuilder)
{
    StringSlicePool pool(StringSlicePool::Style::Empty);
    replaceGetStringHash(module, sharedBuilder, pool);
    addGlobalHashedStringLiterals(pool, sharedBuilder);
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

void addGlobalHashedStringLiterals(const StringSlicePool& pool, SharedIRBuilder& sharedBuilder)
{
    auto slices = pool.getAdded();
    if (slices.getCount() == 0)
    {
        return;
    }

    IRBuilder builder;
    builder.sharedBuilder = &sharedBuilder;
    
    // 
    IRModule* module = builder.getModule();

    // We need to add a global instruction that references all of these string literals
    builder.setInsertInto(module->getModuleInst());

    const Index slicesCount = slices.getCount();

    IRInst* globalHashedInst = createEmptyInst(module, kIROp_GlobalHashedStringLiterals, int(slicesCount));
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

}
