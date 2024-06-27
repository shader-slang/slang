#include "slang-ir-metal-legalize.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-clone.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-parameter-binding.h"
#include "slang-ir-legalize-varying-params.h"

namespace Slang
{
    void legalizeMeshEntryPoint(EntryPointInfo entryPoint)
    {
        auto func = entryPoint.entryPointFunc;

        if (entryPoint.entryPointDecor->getProfile().getStage() != Stage::Mesh)
        {
            return;
        }

        IRBuilder builder{ entryPoint.entryPointFunc->getModule() };
        for (auto param : func->getParams())
        {
            if(param->findDecorationImpl(kIROp_HLSLMeshPayloadDecoration))
            {
                IRVarLayout::Builder varLayoutBuilder(&builder, IRTypeLayout::Builder{&builder}.build());

                varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::MetalPayload);
                auto paramVarLayout = varLayoutBuilder.build();
                builder.addLayoutDecoration(param, paramVarLayout);
            }
        }

    }

    void legalizeDispatchMeshPayloadForMetal(EntryPointInfo entryPoint)
    {
        if (entryPoint.entryPointDecor->getProfile().getStage() != Stage::Amplification)
        {
            return;
        }
        // Find out DispatchMesh function
        IRGlobalValueWithCode* dispatchMeshFunc = nullptr;
        for (const auto globalInst : entryPoint.entryPointFunc->getModule()->getGlobalInsts())
        {
            if (const auto func = as<IRGlobalValueWithCode>(globalInst))
            {
                if (const auto dec = func->findDecoration<IRKnownBuiltinDecoration>())
                {
                    if (dec->getName() == "DispatchMesh")
                    {
                        SLANG_ASSERT(!dispatchMeshFunc && "Multiple DispatchMesh functions found");
                        dispatchMeshFunc = func;
                    }
                }
            }
        }

        if (!dispatchMeshFunc)
            return;

        IRBuilder builder{ entryPoint.entryPointFunc->getModule() };

        // We'll rewrite the call to use mesh_grid_properties.set_threadgroups_per_grid
        traverseUses(dispatchMeshFunc, [&](const IRUse* use) {
            if (const auto call = as<IRCall>(use->getUser()))
            {
                SLANG_ASSERT(call->getArgCount() == 4);
                const auto payload = call->getArg(3);

                const auto payloadPtrType = composeGetters<IRPtrType>(
                    payload,
                    &IRInst::getDataType
                );
                SLANG_ASSERT(payloadPtrType);
                const auto payloadType = payloadPtrType->getValueType();
                SLANG_ASSERT(payloadType);

                builder.setInsertBefore(entryPoint.entryPointFunc->getFirstBlock()->getFirstOrdinaryInst());
                const auto annotatedPayloadType =
                    builder.getPtrType(
                        kIROp_RefType,
                        payloadPtrType->getValueType(),
                        AddressSpace::MetalObjectData
                    );
                auto packedParam = builder.emitParam(annotatedPayloadType);
                builder.addExternCppDecoration(packedParam, toSlice("_slang_mesh_payload"));
                IRVarLayout::Builder varLayoutBuilder(&builder, IRTypeLayout::Builder{&builder}.build());

                // Add the MetalPayload resource info, so we can emit [[payload]]
                varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::MetalPayload);
                auto paramVarLayout = varLayoutBuilder.build();
                builder.addLayoutDecoration(packedParam, paramVarLayout);

                // Now we replace the call to DispatchMesh with a call to the mesh grid properties
                // But first we need to create the parameter
                const auto meshGridPropertiesType = builder.getMetalMeshGridPropertiesType();
                auto mgp = builder.emitParam(meshGridPropertiesType);
                builder.addExternCppDecoration(mgp, toSlice("_slang_mgp"));
                }
            });
    }

    void legalizeEntryPointForMetal(EntryPointInfo entryPoint, DiagnosticSink* sink)
    {
        legalizeMeshEntryPoint(entryPoint);
        legalizeDispatchMeshPayloadForMetal(entryPoint);
    }

    void legalizeFuncBody(IRFunc* func)
    {
        IRBuilder builder(func);
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getModifiableChildren())
            {
                if (auto call = as<IRCall>(inst))
                {
                    ShortList<IRUse*> argsToFixup;
                    // Metal doesn't support taking the address of a vector element.
                    // If such an address is used as an argument to a call, we need to replace it with a temporary.
                    // for example, if we see:
                    // ```
                    //     void foo(inout float x) { x = 1; }
                    //     float4 v;
                    //     foo(v.x);
                    // ```
                    // We need to transform it into:
                    // ```
                    //     float4 v;
                    //     float temp = v.x;
                    //     foo(temp);
                    //     v.x = temp;
                    // ```
                    //
                    for (UInt i = 0; i < call->getArgCount(); i++)
                    {
                        if (auto addr = as<IRGetElementPtr>(call->getArg(i)))
                        {
                            auto ptrType = addr->getBase()->getDataType();
                            auto valueType = tryGetPointedToType(&builder, ptrType);
                            if (!valueType)
                                continue;
                            if (as<IRVectorType>(valueType))
                                argsToFixup.add(call->getArgs() + i);
                        }
                    }
                    if (argsToFixup.getCount() == 0)
                        continue;

                    // Define temp vars for all args that need fixing up.
                    for (auto arg : argsToFixup)
                    {
                        auto addr = as<IRGetElementPtr>(arg->get());
                        auto ptrType = addr->getDataType();
                        auto valueType = tryGetPointedToType(&builder, ptrType);
                        builder.setInsertBefore(call);
                        auto temp = builder.emitVar(valueType);
                        auto initialValue = builder.emitLoad(valueType, addr);
                        builder.emitStore(temp, initialValue);
                        builder.setInsertAfter(call);
                        builder.emitStore(addr, builder.emitLoad(valueType, temp));
                        arg->set(temp);
                    }
                }
            }
        }
    }

    void legalizeIRForMetal(IRModule* module, DiagnosticSink* sink)
    {
        List<EntryPointInfo> entryPoints;
        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
            {
                if (auto entryPointDecor = func->findDecoration<IREntryPointDecoration>())
                {
                    EntryPointInfo info;
                    info.entryPointDecor = entryPointDecor;
                    info.entryPointFunc = func;
                    entryPoints.add(info);
                }
                legalizeFuncBody(func);
            }
        }

        legalizeEntryPointVaryingParamsForMetal(module, sink);
        for (auto entryPoint : entryPoints)
            legalizeEntryPointForMetal(entryPoint, sink);

        specializeAddressSpace(module);
    }

}

