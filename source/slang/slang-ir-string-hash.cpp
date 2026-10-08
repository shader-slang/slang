// slang-ir-string-hash.cpp
#include "slang-ir-string-hash.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

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

static void _addGlobalHashedStringLiteralsToPool(
    IRGlobalHashedStringLiterals* hashedStringLits,
    StringSlicePool& pool)
{
    const Index count = hashedStringLits->getOperandCount();
    for (Index i = 0; i < count; ++i)
    {
        IRStringLit* stringLit = as<IRStringLit>(hashedStringLits->getOperand(i));
        pool.add(stringLit->getStringSlice());
    }
}

static IRGlobalHashedStringLiterals* _findGlobalHashedStringLiterals(IRModule* module)
{
    IRModuleInst* moduleInst = module->getModuleInst();
    IRGlobalHashedStringLiterals* foundInst = nullptr;
    for (IRInst* child : moduleInst->getChildren())
    {
        if (IRGlobalHashedStringLiterals* hashedStringLits =
                as<IRGlobalHashedStringLiterals>(child))
        {
            SLANG_RELEASE_ASSERT(!foundInst || foundInst == hashedStringLits);
            foundInst = hashedStringLits;
        }
    }

    return foundInst;
}

void findGlobalHashedStringLiterals(IRModule* module, StringSlicePool& pool)
{
    auto hashedStringLits = module->_getOrCreateLinkingInfo()->getGlobalHashedStringLiterals();

    if (hashedStringLits)
        _addGlobalHashedStringLiteralsToPool(hashedStringLits, pool);
}

void addGlobalHashedStringLiterals(const StringSlicePool& pool, IRModule* module)
{
    auto slices = pool.getAdded();
    if (slices.getCount() == 0)
    {
        return;
    }

    SLANG_RELEASE_ASSERT(!_findGlobalHashedStringLiterals(module));

    IRBuilder builder(module);

    // We need to add a global instruction that references all of these string literals
    builder.setInsertInto(module->getModuleInst());

    const Index slicesCount = slices.getCount();

    ShortList<IRInst*> operandInsts;
    for (Index i = 0; i < slicesCount; ++i)
    {
        IRStringLit* stringLit = builder.getStringValue(slices[i]);
        operandInsts.add(stringLit);
    }

    auto globalHashedInst = as<IRGlobalHashedStringLiterals>(builder.emitIntrinsicInst(
        nullptr,
        kIROp_GlobalHashedStringLiterals,
        UInt(slicesCount),
        operandInsts.getArrayView().getBuffer()));

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
        // Test the operand directly instead of through `getStringLit()`. That accessor is generated
        // from the typed operand declaration and casts without checking, so it hands back a
        // non-null `IRStringLit*` even when the operand is something else entirely -- which is
        // precisely the case this check exists to reject.
        if (as<IRStringLit>(inst->getOperand(0)) == nullptr)
        {
            if (sink)
            {
                sink->diagnose(Diagnostics::GetStringHashMustBeOnStringLiteral{
                    .location = inst->sourceLoc,
                });
            }

            // Doesn't access a string literal
            return SLANG_FAIL;
        }
    }

    return SLANG_OK;
}

} // namespace Slang
