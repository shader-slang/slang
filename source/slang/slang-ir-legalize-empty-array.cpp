#include "slang-ir-legalize-empty-array.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct EmptyArrayLoweringContext
{
    IRModule* module;
    DiagnosticSink* sink;

    InstWorkList workList;
    InstHashSet workListSet;

    Dictionary<IRInst*, IRInst*> replacements;

    EmptyArrayLoweringContext(IRModule* module)
        : module(module), workList(module), workListSet(module)
    {
    }

    void addToWorkList(IRInst* inst)
    {
        for (auto ii = inst->getParent(); ii; ii = ii->getParent())
        {
            if (as<IRGeneric>(ii))
                return;
        }

        if (workListSet.contains(inst))
            return;

        workList.add(inst);
        workListSet.add(inst);
    }

    bool isEmptyArray(IRType* t)
    {
        const auto lenLit = composeGetters<IRIntLit>(t, &IRArrayType::getElementCount);
        return lenLit ? getIntVal(lenLit) == 0 : false;
    };

    bool hasEmptyArrayType(IRInst* i) { return isEmptyArray(i->getDataType()); }

    bool hasEmptyArrayPtrType(IRInst* i)
    {
        const auto ptr = as<IRPtrTypeBase>(i->getDataType());
        return ptr && isEmptyArray(ptr->getValueType());
    }

    // Visit each instruction and replace it with legalized instructions if necessary.
    void processInst(IRInst* inst)
    {
        //
        // This function is handling several different replacements at once:
        //
        // * Instructions that would create an empty array value are changed
        //   to create a unit value (`void`) instead.
        //
        // * Instruction that would try to read an element from an empty array,
        //   or form a pointer to such an element, instead yield a `poison`
        //   (undefined) value. Note that they do not yield `LoadFromUninitializedMemory`
        //   because there is no guarantee that there would be anything in the
        //   memory corresponding to an index into an empty array.
        //
        // * Instructions that would try to read from an undefined pointer,
        //   buffer, image, etc. yield `poison`.
        //
        // * Instructions that would try to write to an undefined pointer,
        //   buffer, image, etc. are skipped (the behavior is undefined, so
        //   "do nothing" is a valid choice for the behavior, which has the
        //   nice side effect of potentially rendering other code redundant).
        //

        IRInst* replacement = nullptr;

        IRBuilder builder(module);
        builder.setInsertBefore(inst);
        replacement = instMatch<IRInst*>(
            inst,
            nullptr,
            // The following match instructions which take a 0-sized array as an
            // operand and produces a result value for the inst.
            [&](IRGetElement* getElement)
            {
                const auto base = getElement->getBase();
                return hasEmptyArrayType(base) ? builder.emitPoison(getElement->getDataType())
                                               : nullptr;
            },
            [&](IRGetElementPtr* gep)
            {
                const auto base = gep->getBase();
                return hasEmptyArrayPtrType(gep) || hasEmptyArrayPtrType(base) ||
                               as<IRUndefined>(base)
                           ? builder.emitPoison(gep->getDataType())
                           : nullptr;
            },
            [&](IRFieldAddress* gep)
            {
                const auto base = gep->getBase();
                return hasEmptyArrayPtrType(gep) || as<IRUndefined>(base)
                           ? builder.emitPoison(gep->getDataType())
                           : nullptr;
            },
            [&](IRLoad* load)
            {
                return as<IRUndefined>(load->getOperand(0))
                           ? builder.emitPoison(load->getDataType())
                           : nullptr;
            },
            [&](IRImageLoad* load)
            {
                return as<IRUndefined>(load->getOperand(0))
                           ? builder.emitPoison(load->getDataType())
                           : nullptr;
            },
            [&](IRStore* store)
            {
                if (as<IRUndefined>(store->getPtr()))
                    store->removeAndDeallocate();
                return nullptr;
            },
            [&](IRAtomicStore* store)
            {
                if (as<IRUndefined>(store->getPtr()))
                    store->removeAndDeallocate();
                return nullptr;
            },
            [&](IRImageStore* store)
            {
                if (as<IRUndefined>(store->getImage()))
                    store->removeAndDeallocate();
                return nullptr;
            },
            [&](IRImageSubscript* subscript)
            {
                return as<IRUndefined>(subscript->getImage())
                           ? builder.emitPoison(subscript->getDataType())
                           : nullptr;
            },
            [&](IRAtomicOperation* atomic)
            {
                return as<IRUndefined>(atomic->getOperand(0))
                           ? builder.emitPoison(atomic->getDataType())
                           : nullptr;
            },
            // The following should match any instruction which can construct a 0-sized array.
            [&](IRMakeArray* makeArray) {
                return hasEmptyArrayType(makeArray->getDataType()) ? builder.getVoidValue()
                                                                   : nullptr;
            },
            [&](IRMakeArrayFromElement* makeArray) {
                return hasEmptyArrayType(makeArray->getDataType()) ? builder.getVoidValue()
                                                                   : nullptr;
            });

        // If we did get a replacement, add that to our mapping and return
        // it, otherwise return the original (to maybe be updated later)
        if (replacement)
        {
            inst->replaceUsesWith(replacement);
            inst->removeAndDeallocate();
            addToWorkList(replacement);
            for (auto use = replacement->firstUse; use; use = use->nextUse)
            {
                addToWorkList(use->getUser());
            }
        }
        else if (isEmptyArray((IRType*)inst))
        {
            replacements.add(inst, builder.getVoidType());
        }
    }

    void processModule()
    {
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.remove(inst);

            // Run this inst through the replacer
            processInst(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }

        // Apply all deferred replacements
        //
        // It's important to defer this as if we were updating things
        // on-the-fly we would be losing information about what was
        // actually a 0-array or not.
        for (const auto& [old, replacement] : replacements)
        {
            if (old != replacement)
            {
                old->replaceUsesWith(replacement);
                old->removeAndDeallocate();
            }
        }
    }
};

void legalizeEmptyArray(IRModule* module, DiagnosticSink* sink)
{
    EmptyArrayLoweringContext context(module);
    context.sink = sink;
    context.processModule();
}
} // namespace Slang
