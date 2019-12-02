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

void addGlobalHashedStringLiterals(const HashSet<IRStringLit*>& hashSet, IRBuilder& ioBuilder)
{
    // Add the list of all of the instructions 
    const Index stringLitCount = Index(hashSet.Count());
    if (stringLitCount == 0)
    {
        return;
    }

    IRModule* module = ioBuilder.getModule();

    // We need to add a global instruction that references all of these string literals
    ioBuilder.setInsertInto(module->getModuleInst());

    IRInst* globalHashedInst = createEmptyInst(module, kIROp_GlobalHashedStringLiterals, int(hashSet.Count()));

    Index index = 0;
    for (auto stringLit : hashSet)
    {
        SLANG_ASSERT(index < Index(hashSet.Count()));
        globalHashedInst->getOperands()[index].set(stringLit);
        index++;
    }

    ioBuilder.addInst(globalHashedInst);

    // Mark to keep alive
    ioBuilder.addKeepAliveDecoration(globalHashedInst);
}

SlangResult replaceGetStringHash(IRModule* module)
{
    List<IRGetStringHash*> insts;
    _findGetStringHashRec(module->getModuleInst(), insts);

    HashSet<IRStringLit*> hashSet;

    SharedIRBuilder sharedBuilder;

    sharedBuilder.module = module;
    sharedBuilder.session = module->getSession();

    IRBuilder builder;
    builder.sharedBuilder = &sharedBuilder;

    // Then we want to add the GlobalHashedString instruction in the root
    for (auto inst : insts)
    {
        IRStringLit* stringLit = inst->getStringLit();
        hashSet.Add(stringLit);

        // Okay work out what the hash is
        const int hash = GetHashCode(stringLit->getStringSlice());

        IRInst* intLit = builder.getIntValue(builder.getIntType(), int32_t(hash));

        // Okay we want to replace all uses with the literal
        inst->replaceUsesWith(intLit);
        inst->removeAndDeallocate();
    }

    addGlobalHashedStringLiterals(hashSet, builder);

    return SLANG_OK;
}

}
