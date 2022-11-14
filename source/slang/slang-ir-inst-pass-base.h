// slang-ir-inst-pass-base.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{
    struct IRModule;

    class InstPassBase
    {
    protected:
        IRModule* module;
        SharedIRBuilder sharedBuilderStorage;

        List<IRInst*> workList;
        HashSet<IRInst*> workListSet;
        void addToWorkList(IRInst* inst)
        {
            if (workListSet.Contains(inst))
                return;

            workList.add(inst);
            workListSet.Add(inst);
        }

        IRInst* pop()
        {
            if (workList.getCount() == 0)
                return nullptr;

            IRInst* inst = workList.getLast();
            workList.removeLast();
            workListSet.Remove(inst);
            return inst;
        }

    public:
        InstPassBase(IRModule* inModule)
            : module(inModule)
        {}

        template <typename InstType, typename Func>
        void processInstsOfType(IROp instOp, const Func& f)
        {
            workList.clear();
            workListSet.Clear();

            addToWorkList(module->getModuleInst());

            while (workList.getCount() != 0)
            {
                IRInst* inst = pop();

                if (inst->getOp() == instOp)
                {
                    f(as<InstType>(inst));
                }

                for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                {
                    addToWorkList(child);
                }
            }
        }

        template <typename InstType, typename Func>
        void processChildInstsOfType(IROp instOp, IRInst* parent, const Func& f)
        {
            workList.clear();
            workListSet.Clear();

            addToWorkList(parent);

            while (workList.getCount() != 0)
            {
                IRInst* inst = pop();
                if (inst->getOp() == instOp)
                {
                    f(as<InstType>(inst));
                }

                for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                {
                    addToWorkList(child);
                }
            }
        }

        template <typename Func>
        void processAllInsts(const Func& f)
        {
            workList.clear();
            workListSet.Clear();

            addToWorkList(module->getModuleInst());

            while (workList.getCount() != 0)
            {
                IRInst* inst = pop();

                f(inst);

                for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                {
                    addToWorkList(child);
                }
            }
        }
    };

}
