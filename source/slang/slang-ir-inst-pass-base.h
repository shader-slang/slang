// slang-ir-inst-pass-base.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-dce.h"

namespace Slang
{
    struct IRModule;

    class InstPassBase
    {
    protected:
        IRModule* module;
        List<IRInst*> workList;
        HashSet<IRInst*> workListSet;
        void addToWorkList(IRInst* inst)
        {
            if (workListSet.Contains(inst))
                return;

            workList.add(inst);
            workListSet.Add(inst);
        }

        IRInst* pop(bool removeFromSet = true)
        {
            if (workList.getCount() == 0)
                return nullptr;

            IRInst* inst = workList.getLast();
            workList.removeLast();
            if (removeFromSet)
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
        void processChildInsts(IRInst* root, const Func& f)
        {
            workList.clear();
            workListSet.Clear();

            addToWorkList(root);

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

        template <typename Func>
        void processAllInsts(const Func& f)
        {
            processChildInsts(module->getModuleInst(), f);
        }

        template <typename Func>
        void processAllReachableInsts(const Func& f)
        {
            workList.clear();
            workListSet.Clear();

            addToWorkList(module->getModuleInst());
            while (workList.getCount() != 0)
            {
                IRInst* inst = pop(false);
                f(inst);
                for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                {
                    if (as<IRDecoration>(child))
                        break;
                    switch (child->getOp())
                    {
                    case kIROp_GenericSpecializationDictionary:
                    case kIROp_ExistentialFuncSpecializationDictionary:
                    case kIROp_ExistentialTypeSpecializationDictionary:
                        continue;
                    default:
                        break;
                    }
                    if (shouldInstBeLiveIfParentIsLive(child, IRDeadCodeEliminationOptions()))
                        addToWorkList(child);
                }
            }
        }
    };

}
