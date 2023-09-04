#include "slang-ir-legalize-vector-types.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
    struct VectorTypeLoweringContext
    {
        IRModule* module;
        DiagnosticSink* sink;

        InstWorkList workList;
        InstHashSet workListSet;

        VectorTypeLoweringContext(IRModule* module)
            :module(module), workList(module), workListSet(module)
        {}

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

        void process1VectorPtrType(IRPtrTypeBase* ptr)
        {
            IRBuilder builder(module);
            const auto vec1Type = cast<IRVectorType>(ptr->getValueType());
            const auto scalarType = vec1Type->getElementType();
            const auto scalarPtr = builder.getPtrType(ptr->getOp(), scalarType, ptr->getAddressSpace());

            traverseUses(ptr, [&](IRUse* use){
                const auto user = use->user;

                // Types will be updated later when we replaceUsesWith
                if(as<IRType>(user))
                    return;

                InstHashSet vec1PtrConsumers(module);

                if(user->getDataType() == ptr)
                {
                    for(auto u = user->firstUse; u; u = u->nextUse)
                        if(!vec1PtrConsumers.contains(u->getUser()))
                            vec1PtrConsumers.add(u->getUser());
                }

                switch(user->getOp())
                {
                case kIROp_FieldAddress:
                {
                    // const auto fa = cast<IRFieldAddress>(user);
                    // const auto scalar = fa->getBase();
                    // user->replaceUsesWith(scalar);
                    // user->removeAndDeallocate();
                }
                break;
                case kIROp_GetElementPtr:
                // {
                //     const auto fa = cast<IRGetElementPtr>(user);
                //     const auto scalar = fa->getBase();
                //     user->replaceUsesWith(scalar);
                //     user->removeAndDeallocate();
                // }
                break;
                case kIROp_Var:
                {
                    builder.setInsertBefore(user);
                    const auto scalarVar = builder.emitVar(scalarType);
                    user->replaceUsesWith(scalarVar);
                    user->removeAndDeallocate();
                }
                break;

                // Turn this on to easily find more cases to add
#               if 1
                default:
                {
                    String e = "Missing case in process1PtrVectorType:\n"
                        + dumpIRToString(user, {IRDumpOptions::Mode::Detailed, 0});
                    SLANG_UNIMPLEMENTED_X(e.begin());
                }
#               endif
                }

                //
                // Now we handle the uses of all ptr<vec1> typed things
                //
                for(const auto vec1PtrConsumer : *vec1PtrConsumers.set)
                {
                    switch(vec1PtrConsumer->getOp())
                    {
                    case kIROp_GetElementPtr:
                    {
                        const auto fa = cast<IRGetElementPtr>(vec1PtrConsumer);
                        const auto scalar = fa->getBase();
                        vec1PtrConsumer->replaceUsesWith(scalar);
                        vec1PtrConsumer->removeAndDeallocate();
                    }

                    // Turn this on to easily find more cases to add
#               if 0
                    default:
                    {
                        String e = "Missing case in process1VectorPtrType vec1PtrValUser:\n"
                            + dumpIRToString(vec1PtrConsumer, {IRDumpOptions::Mode::Detailed, 0});
                        SLANG_UNIMPLEMENTED_X(e.begin());
                    }
#endif
                    }
                }
            });
        }

        void process1VectorType(IRType* vecTy)
        {
            InstHashSet vec1Consumers(module);

            const auto vecDataTy
                = as<IRVectorType>(vecTy) ? cast<IRVectorType>(vecTy)
                : as<IRRateQualifiedType>(vecTy) ? as<IRVectorType>(cast<IRRateQualifiedType>(vecTy)->getValueType())
                : nullptr;

            SLANG_ASSERT(vecDataTy);

            IRBuilder builder(module);
            auto scalarType = vecDataTy->getElementType();
            if(const auto r = as<IRRateQualifiedType>(vecTy))
                scalarType = builder.getRateQualifiedType(r->getRate(), scalarType);

            traverseUses(vecTy, [&](IRUse* use){
                const auto user = use->user;

                if(const auto v1Ptr = as<IRPtrTypeBase>(user))
                    process1VectorPtrType(v1Ptr);

                if(const auto rateType = as<IRRateQualifiedType>(user))
                    return process1VectorType(rateType);

                // Types will be updated when we replaceUsesWith
                if(as<IRType>(user))
                    return;

                //
                // If this instruction produces a 1-vector, add its uses to the
                // consumer list
                //
                if(user->getDataType() == vecTy)
                {
                    for(auto u = user->firstUse; u; u = u->nextUse)
                        if(!vec1Consumers.contains(u->getUser()))
                            vec1Consumers.add(u->getUser());
                }

                switch(user->getOp())
                {
                case kIROp_SPIRVAsmOperandInst:
                    // TODO: Assume that the spirv_asm writer knows what they're doing
                break;

                // TODO: Handle more things here which create vectors

                //
                // Positive uses, a value of this type is being created
                //
                case kIROp_MakeVectorFromScalar:
                case kIROp_MakeVector:
                {
                    const auto scalar = user->getOperand(0);
                    user->replaceUsesWith(scalar);
                    user->removeAndDeallocate();
                }
                break;

                // Turn this on to easily find more cases to add
#               if 0
                default:
                {
                    String e = "Missing case in process1VectorType:\n"
                        + dumpIRToString(user, {IRDumpOptions::Mode::Detailed, 0});
                    SLANG_UNIMPLEMENTED_X(e.begin());
                }
#               endif
                }
            });

            //
            // Now we handle the uses of all vec1 typed things
            //
            for(const auto vec1Consumer : *vec1Consumers.set)
            {
                switch(vec1Consumer->getOp())
                {
                case kIROp_SPIRVAsmOperandInst:
                    // TODO: Assume that the spirv_asm writer knows what they're doing
                break;

                // TODO: handle things here such as getElement, swizzle etc...
                case kIROp_GetElementPtr:
                case kIROp_SwizzledStore:
                    SLANG_UNIMPLEMENTED_X("Vector user in 1-vector legalization");

                case kIROp_GetElement:
                {
                    const auto ge = cast<IRGetElement>(vec1Consumer);
                    const auto scalar = ge->getBase();
                    ge->replaceUsesWith(scalar);
                    ge->removeAndDeallocate();
                }
                break;

                case kIROp_swizzle:
                {
                    const auto swizzle = as<IRSwizzle>(vec1Consumer);
                    const auto swizzleLength = swizzle->getElementCount();
                    if(swizzleLength == 1)
                    {
                        swizzle->replaceUsesWith(swizzle->getBase());
                    }
                    else
                    {
                        builder.setInsertBefore(swizzle);
                        // TODO: This isn't type-correct at this stage...
                        const auto v = builder.emitMakeVectorFromScalar(
                            vec1Consumer->getFullType(),
                            swizzle->getBase());
                        swizzle->replaceUsesWith(v);
                    }
                    swizzle->removeAndDeallocate();
                }
                break;

                // Turn this on to easily find more cases to add
#               if 0
                default:
                {
                    String e = "Missing case in process1VectorType vec1ValUser:\n"
                        + dumpIRToString(vec1Consumer, {IRDumpOptions::Mode::Detailed, 0});
                    SLANG_UNIMPLEMENTED_X(e.begin());
                }
#endif
                }
            }

            vecTy->replaceUsesWith(scalarType);
            vecTy->removeAndDeallocate();
        }

        void processInst(IRInst* inst)
        {
            switch (inst->getOp())
            {
            case kIROp_VectorType:
            {
                const auto vec = cast<IRVectorType>(inst);
                const auto lenInst = vec->getElementCount();
                if(const auto lenLit = as<IRIntLit>(lenInst))
                {
                    const auto len = getIntVal(lenLit);
                    if(len == 1)
                        process1VectorType(vec);
                }
                break;
            }
            default:
                break;
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

                processInst(inst);

                for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                {
                    addToWorkList(child);
                }
            }
        }
    };

    void legalizeVectorTypes(IRModule* module, DiagnosticSink* sink)
    {
        VectorTypeLoweringContext context(module);
        context.sink = sink;
        context.processModule();
    }
}
