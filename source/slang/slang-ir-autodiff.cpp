#include "slang-ir-autodiff.h"
#include "slang-ir-autodiff-rev.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-pairs.h"

namespace Slang
{
static IRInst* _lookupWitness(IRBuilder* builder, IRInst* witness, IRInst* requirementKey)
{
    if (auto witnessTable = as<IRWitnessTable>(witness))
    {
        for (auto entry : witnessTable->getEntries())
        {
            if (entry->getRequirementKey() == requirementKey)
                return entry->getSatisfyingVal();
        }
    }
    else if (auto witnessTableParam = as<IRParam>(witness))
    {
        return builder->emitLookupInterfaceMethodInst(
            builder->getTypeKind(),
            witnessTableParam,
            requirementKey);
    }
    return nullptr;
}

bool isNoDiffType(IRType* paramType)
{
    while (auto ptrType = as<IRPtrTypeBase>(paramType))
        paramType = ptrType->getValueType();
    while (auto attrType = as<IRAttributedType>(paramType))
    {
        if (attrType->findAttr<IRNoDiffAttr>())
        {
            return true;
        }
    }
    return false;
}

IRInst* lookupForwardDerivativeReference(IRInst* primalFunction)
{
    if (auto jvpDefinition = primalFunction->findDecoration<IRForwardDerivativeDecoration>())
        return jvpDefinition->getForwardDerivativeFunc();
    return nullptr;
}

IRStructField* DifferentialPairTypeBuilder::findField(IRInst* type, IRStructKey* key)
{
    if (auto irStructType = as<IRStructType>(type))
    {
        for (auto field : irStructType->getFields())
        {
            if (field->getKey() == key)
            {
                return field;
            }
        }
    }
    else if (auto irSpecialize = as<IRSpecialize>(type))
    {
        if (auto irGeneric = as<IRGeneric>(irSpecialize->getBase()))
        {
            if (auto irGenericStructType = as<IRStructType>(findInnerMostGenericReturnVal(irGeneric)))
            {
                return findField(irGenericStructType, key);
            }
        }
    }

    return nullptr;
}

IRInst* DifferentialPairTypeBuilder::findSpecializationForParam(IRInst* specializeInst, IRInst* genericParam)
{
    // Get base generic that's being specialized.
    auto genericType = as<IRGeneric>(as<IRSpecialize>(specializeInst)->getBase());
    SLANG_ASSERT(genericType);
    
    // Find the index of genericParam in the base generic.
    int paramIndex = -1;
    int currentIndex = 0;
    for (auto param : genericType->getParams())
    {
        if (param == genericParam)
            paramIndex = currentIndex;
        currentIndex ++;
    }

    SLANG_ASSERT(paramIndex >= 0);

    // Return the corresponding operand in the specialization inst.
    return specializeInst->getOperand(1 + paramIndex);
}

IRInst* DifferentialPairTypeBuilder::emitFieldAccessor(IRBuilder* builder, IRInst* baseInst, IRStructKey* key)
{
    IRInst* pairType = nullptr;
    if (auto basePtrType = as<IRPtrTypeBase>(baseInst->getDataType()))
    {
        auto loweredType = lowerDiffPairType(builder, basePtrType->getValueType());

        pairType = builder->getPtrType(kIROp_PtrType, (IRType*)loweredType);
    }
    else
    {
        auto loweredType = lowerDiffPairType(builder, baseInst->getDataType());
        pairType = loweredType;
    }

    if (auto basePairStructType = as<IRStructType>(pairType))
    {
        return as<IRFieldExtract>(builder->emitFieldExtract(
                findField(basePairStructType, key)->getFieldType(),
                baseInst,
                key
            ));
    }
    else if (auto ptrType = as<IRPtrTypeBase>(pairType))
    {
        if (auto ptrInnerSpecializedType = as<IRSpecialize>(ptrType->getValueType()))
        {
            auto genericType = findInnerMostGenericReturnVal(as<IRGeneric>(ptrInnerSpecializedType->getBase()));
            if (auto genericBasePairStructType = as<IRStructType>(genericType))
            {
                return as<IRFieldAddress>(builder->emitFieldAddress(
                    builder->getPtrType((IRType*)
                        findSpecializationForParam(
                            ptrInnerSpecializedType,
                            findField(ptrInnerSpecializedType, key)->getFieldType())),
                    baseInst,
                    key
                ));
            }
        }
        else if (auto ptrBaseStructType = as<IRStructType>(ptrType->getValueType()))
        {
            return as<IRFieldAddress>(builder->emitFieldAddress(
                builder->getPtrType((IRType*)
                        findField(ptrBaseStructType, key)->getFieldType()),
                baseInst,
                key));
        }
    }
    else if (auto specializedType = as<IRSpecialize>(pairType))
    {
        // TODO: Stopped here -> The type being emitted is incorrect. don't emit the generic's
        // type, emit the specialization type.
        // 
        auto genericType = findInnerMostGenericReturnVal(as<IRGeneric>(specializedType->getBase()));
        if (auto genericBasePairStructType = as<IRStructType>(genericType))
        {
            return as<IRFieldExtract>(builder->emitFieldExtract(
                (IRType*)findSpecializationForParam(
                    specializedType,
                    findField(genericBasePairStructType, key)->getFieldType()),
                baseInst,
                key
            ));
        }
        else if (auto genericPtrType = as<IRPtrTypeBase>(genericType))
        {
            if (auto genericPairStructType = as<IRStructType>(genericPtrType->getValueType()))
            {
                return as<IRFieldAddress>(builder->emitFieldAddress(
                        builder->getPtrType((IRType*)
                            findSpecializationForParam(
                                specializedType,
                                findField(genericPairStructType, key)->getFieldType())),
                        baseInst,
                        key
                    ));
            }
        }
    }
    else
    {
        SLANG_UNEXPECTED("Unrecognized field. Cannot emit field accessor");
    }
    return nullptr;
}

IRInst* DifferentialPairTypeBuilder::emitPrimalFieldAccess(IRBuilder* builder, IRInst* baseInst)
{
    return emitFieldAccessor(builder, baseInst, this->globalPrimalKey);
}

IRInst* DifferentialPairTypeBuilder::emitDiffFieldAccess(IRBuilder* builder, IRInst* baseInst)
{
    return emitFieldAccessor(builder, baseInst, this->globalDiffKey);
}

IRStructKey* DifferentialPairTypeBuilder::_getOrCreateDiffStructKey()
{
    if (!this->globalDiffKey)
    {
        IRBuilder builder(sharedContext->sharedBuilder);
        // Insert directly at top level (skip any generic scopes etc.)
        builder.setInsertInto(sharedContext->moduleInst);

        this->globalDiffKey = builder.createStructKey();
        builder.addNameHintDecoration(this->globalDiffKey , UnownedTerminatedStringSlice("differential"));
    }

    return this->globalDiffKey;
}

IRStructKey* DifferentialPairTypeBuilder::_getOrCreatePrimalStructKey()
{
    if (!this->globalPrimalKey)
    {
        // Insert directly at top level (skip any generic scopes etc.)
        IRBuilder builder(sharedContext->sharedBuilder);
        builder.setInsertInto(sharedContext->moduleInst);

        this->globalPrimalKey = builder.createStructKey();
        builder.addNameHintDecoration(this->globalPrimalKey , UnownedTerminatedStringSlice("primal"));
    }

    return this->globalPrimalKey;
}

IRInst* DifferentialPairTypeBuilder::_createDiffPairType(IRType* origBaseType, IRType* diffType)
{
    switch (origBaseType->getOp())
    {
    case kIROp_LookupWitness:
    case kIROp_Specialize:
    case kIROp_Param:
        return nullptr;
    default:
        break;
    }

    IRBuilder builder(sharedContext->sharedBuilder);
    builder.setInsertBefore(diffType);

    auto pairStructType = builder.createStructType();
    builder.createStructField(pairStructType, _getOrCreatePrimalStructKey(), origBaseType);
    builder.createStructField(pairStructType, _getOrCreateDiffStructKey(), (IRType*)diffType);
    return pairStructType;
}

IRInst* DifferentialPairTypeBuilder::getDiffTypeFromPairType(IRBuilder* builder, IRDifferentialPairType* type)
{
    auto witnessTable = type->getWitness();
    return _lookupWitness(builder, witnessTable, sharedContext->differentialAssocTypeStructKey);
}

IRInst* DifferentialPairTypeBuilder::getDiffTypeWitnessFromPairType(IRBuilder* builder, IRDifferentialPairType* type)
{
    auto witnessTable = type->getWitness();
    return _lookupWitness(builder, witnessTable, sharedContext->differentialAssocTypeWitnessStructKey);
}

IRInst* DifferentialPairTypeBuilder::lowerDiffPairType(
    IRBuilder* builder, IRType* originalPairType)
{
    IRInst* result = nullptr;
    if (pairTypeCache.TryGetValue(originalPairType, result))
        return result;
    auto pairType = as<IRDifferentialPairType>(originalPairType);
    if (!pairType)
    {
        result = originalPairType;
        return result;
    }
    auto primalType = pairType->getValueType();
    if (as<IRParam>(primalType))
    {
        result = nullptr;
        return result;
    }

    auto diffType = getDiffTypeFromPairType(builder, pairType);
    if (!diffType)
        return result;
    result = _createDiffPairType(pairType->getValueType(), (IRType*)diffType);
    pairTypeCache.Add(originalPairType, result);

    return result;
}

AutoDiffSharedContext::AutoDiffSharedContext(IRModuleInst* inModuleInst)
    : moduleInst(inModuleInst)
{
    differentiableInterfaceType = as<IRInterfaceType>(findDifferentiableInterface());
    if (differentiableInterfaceType)
    {
        differentialAssocTypeStructKey = findDifferentialTypeStructKey();
        differentialAssocTypeWitnessStructKey = findDifferentialTypeWitnessStructKey();
        zeroMethodStructKey = findZeroMethodStructKey();
        addMethodStructKey = findAddMethodStructKey();
        mulMethodStructKey = findMulMethodStructKey();

        if (differentialAssocTypeStructKey)
            isInterfaceAvailable = true;
    }
}

IRInst* AutoDiffSharedContext::findDifferentiableInterface()
{
    if (auto module = as<IRModuleInst>(moduleInst))
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            // TODO: This seems like a particularly dangerous way to look for an interface.
            // See if we can lower IDifferentiable to a separate IR inst.
            //
            if (globalInst->getOp() == kIROp_InterfaceType &&
                as<IRInterfaceType>(globalInst)->findDecoration<IRNameHintDecoration>()->getName() == "IDifferentiable")
            {
                return globalInst;
            }
        }
    }
    return nullptr;
}

IRStructKey* AutoDiffSharedContext::getIDifferentiableStructKeyAtIndex(UInt index)
{
    if (as<IRModuleInst>(moduleInst) && differentiableInterfaceType)
    {
        // Assume for now that IDifferentiable has exactly five fields.
        SLANG_ASSERT(differentiableInterfaceType->getOperandCount() == 5);
        if (auto entry = as<IRInterfaceRequirementEntry>(differentiableInterfaceType->getOperand(index)))
            return as<IRStructKey>(entry->getRequirementKey());
        else
        {
            SLANG_UNEXPECTED("IDifferentiable interface entry unexpected type");
        }
    }

    return nullptr;
}

void DifferentiableTypeConformanceContext::setFunc(IRGlobalValueWithCode* func)
{
    parentFunc = func;

    auto decor = func->findDecoration<IRDifferentiableTypeDictionaryDecoration>();
    SLANG_RELEASE_ASSERT(decor);

    // Build lookup dictionary for type witnesses.
    for (auto child = decor->getFirstChild(); child; child = child->next)
    {
        if (auto item = as<IRDifferentiableTypeDictionaryItem>(child))
        {
            auto existingItem = differentiableWitnessDictionary.TryGetValue(item->getConcreteType());
            if (existingItem)
            {
                *existingItem = item->getWitness();
            }
            else
            {
                differentiableWitnessDictionary.Add((IRType*)item->getConcreteType(), item->getWitness());
            }
        }
    }
}

IRInst* DifferentiableTypeConformanceContext::lookUpConformanceForType(IRInst* type)
{
    IRInst* foundResult = nullptr;
    differentiableWitnessDictionary.TryGetValue(type, foundResult);
    return foundResult;
}

IRInst* DifferentiableTypeConformanceContext::lookUpInterfaceMethod(IRBuilder* builder, IRType* origType, IRStructKey* key)
{
    if (auto conformance = lookUpConformanceForType(origType))
    {
        return _lookupWitness(builder, conformance, key);
    }
    return nullptr;
}

void DifferentiableTypeConformanceContext::buildGlobalWitnessDictionary()
{
    for (auto globalInst : sharedContext->moduleInst->getChildren())
    {
        if (auto pairType = as<IRDifferentialPairType>(globalInst))
        {
            differentiableWitnessDictionary.Add(pairType->getValueType(), pairType->getWitness());
        }
    }
}

void stripAutoDiffDecorationsFromChildren(IRInst* parent)
{
    for (auto inst : parent->getChildren())
    {
        for (auto decor = inst->getFirstDecoration(); decor; )
        {
            auto next = decor->getNextDecoration();
            switch (decor->getOp())
            {
            case kIROp_ForwardDerivativeDecoration:
            case kIROp_DerivativeMemberDecoration:
            case kIROp_DifferentiableTypeDictionaryDecoration:
            case kIROp_DifferentialInstDecoration:
            case kIROp_MixedDifferentialInstDecoration:
                decor->removeAndDeallocate();
                break;
            default:
                break;
            }
            decor = next;
        }

        if (inst->getFirstChild() != nullptr)
        {
            stripAutoDiffDecorationsFromChildren(inst);
        }
    }
}

void stripAutoDiffDecorations(IRModule* module)
{
    stripAutoDiffDecorationsFromChildren(module->getModuleInst());
}


void stripBlockTypeDecorations(IRFunc* func)
{
    for (auto child : func->getChildren())
    {
        if (auto block = as<IRBlock>(child))
        {
            for (auto decor = block->getFirstDecoration(); decor; )
            {
                auto next = decor->getNextDecoration();
                switch (decor->getOp())
                {
                case kIROp_DifferentialInstDecoration:
                case kIROp_MixedDifferentialInstDecoration:
                    decor->removeAndDeallocate();
                    break;
                default:
                    break;
                }
                decor = next;
            }
        }
    }
}


struct StripNoDiffTypeAttributePass : InstPassBase
{
    StripNoDiffTypeAttributePass(IRModule* module) :
        InstPassBase(module)
    {
    }
    void processModule()
    {
        processInstsOfType<IRAttributedType>(kIROp_AttributedType, [&](IRAttributedType* attrType)
            {
                if (attrType->getAllAttrs().getCount() == 1)
                {
                    if (attrType->findAttr<IRNoDiffAttr>())
                    {
                        attrType->replaceUsesWith(attrType->getBaseType());
                        attrType->removeAndDeallocate();
                    }
                }
            });
        sharedBuilderStorage.init(module);
        sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();
    }
};

void stripNoDiffTypeAttribute(IRModule* module)
{
    StripNoDiffTypeAttributePass pass(module);
    pass.processModule();
}

struct AutoDiffPass : public InstPassBase
{
    DiagnosticSink* getSink()
    {
        return sink;
    }

    bool processModule()
    {
        // TODO(sai): Move this call.
        forwardTranscriber.differentiableTypeConformanceContext.buildGlobalWitnessDictionary();

        IRBuilder builderStorage(this->autodiffContext->sharedBuilder);
        IRBuilder* builder = &builderStorage;

        // Process all ForwardDifferentiate and BackwardDifferentiate instructions by 
        // generating derivative code for the referenced function.
        //
        bool modified = processReferencedFunctions(builder);

        return modified;
    }

    // Process all differentiate calls, and recursively generate code for forward and backward
    // derivative functions.
    //
    bool processReferencedFunctions(IRBuilder* builder)
    {
        bool hasChanges = false;
        for (;;)
        {
            bool changed = false;
            List<IRInst*> autoDiffWorkList;
            // Collect all `ForwardDifferentiate`/`BackwardDifferentiate` insts from the module.
            autoDiffWorkList.clear();
            processAllInsts([&](IRInst* inst)
                {
                    switch (inst->getOp())
                    {
                    case kIROp_ForwardDifferentiate:
                    case kIROp_BackwardDifferentiate:
                        // Only process now if the operand is a materialized function.
                        switch (inst->getOperand(0)->getOp())
                        {
                        case kIROp_Func:
                        case kIROp_Specialize:
                        case kIROp_LookupWitness:
                            autoDiffWorkList.add(inst);
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                });

            // Process collected differentiate insts and replace them with placeholders for
            // differentiated functions.

            for (auto differentiateInst : autoDiffWorkList)
            {
                if (auto diffInst = as<IRForwardDifferentiate>(differentiateInst))
                {
                    IRBuilder subBuilder(*builder);
                    subBuilder.setInsertBefore(differentiateInst);
                    if (auto diffFunc = forwardTranscriber.transcribe(&subBuilder, diffInst->getBaseFn()))
                    {
                        differentiateInst->replaceUsesWith(diffFunc);
                        differentiateInst->removeAndDeallocate();
                        changed = true;
                    }
                }
                else if (auto backDiffInst = as<IRBackwardDifferentiate>(differentiateInst))
                {
                    auto baseInst = backDiffInst->getBaseFn();
                    if (auto diffFunc = backwardTranscriber.transcribe(builder, (IRFunc*)baseInst))
                    {
                        SLANG_ASSERT(diffFunc);
                        differentiateInst->replaceUsesWith(diffFunc);
                        differentiateInst->removeAndDeallocate();
                        changed = true;
                    }
                }
            }

            // Run transcription logic to generate the body of forward/backward derivatives functions.
            // While doing so, we may discover new functions to differentiate, so we keep running until
            // the worklist goes dry.
            List<IRFunc*> autodiffCleanupList;
            while (autodiffContext->followUpFunctionsToTranscribe.getCount() != 0)
            {
                changed = true;
                auto followUpWorkList = _Move(autodiffContext->followUpFunctionsToTranscribe);
                for (auto task : followUpWorkList)
                {
                    auto diffFunc = as<IRFunc>(task.resultFunc);
                    SLANG_ASSERT(diffFunc);

                    // We're running in to some situations where the follow-up task
                    // has already been completed (diffFunc has been generated, processed,
                    // and deallocated). Skip over these for now.
                    // 
                    if (!diffFunc->getDataType())
                        continue;

                    auto primalFunc = as<IRFunc>(task.originalFunc);
                    SLANG_ASSERT(primalFunc);
                    switch (task.type)
                    {
                    case FuncBodyTranscriptionTaskType::Forward:
                        forwardTranscriber.transcribeFunc(builder, primalFunc, diffFunc);
                        break;
                    case FuncBodyTranscriptionTaskType::Backward:
                        backwardTranscriber.transcribeFunc(builder, primalFunc, diffFunc);
                        break;
                    default:
                        break;
                    }

                    autodiffCleanupList.add(diffFunc);
                }
            }

            // Get rid of block-level decorations that are used to keep track of 
            // different block types. These don't work well with the IR simplification
            // passes since they don't expect decorations in blocks.
            // 
            for (auto diffFunc : autodiffCleanupList)
                stripBlockTypeDecorations(diffFunc);

            autodiffCleanupList.clear();

            if (!changed)
                break;
            hasChanges |= changed;
        }


        return hasChanges;
    }

    IRStringLit* getDerivativeFuncName(IRInst* func, const char* postFix)
    {
        IRBuilder builder(&sharedBuilderStorage);
        builder.setInsertBefore(func);

        IRStringLit* name = nullptr;
        if (auto linkageDecoration = func->findDecoration<IRLinkageDecoration>())
        {
            name = builder.getStringValue((String(linkageDecoration->getMangledName()) + postFix).getUnownedSlice());
        }
        else if (auto namehintDecoration = func->findDecoration<IRNameHintDecoration>())
        {
            name = builder.getStringValue((String(namehintDecoration->getName()) + postFix).getUnownedSlice());
        }

        return name;
    }

    IRStringLit* getForwardDerivativeFuncName(IRInst* func)
    {
        return getDerivativeFuncName(func, "_fwd_diff");
    }

    IRStringLit* getBackwardDerivativeFuncName(IRInst* func)
    {
        return getDerivativeFuncName(func, "_bwd_diff");
    }

    AutoDiffPass(AutoDiffSharedContext* context, DiagnosticSink* sink) :
        InstPassBase(context->moduleInst->getModule()),
        sink(sink),
        forwardTranscriber(context, context->sharedBuilder, sink),
        backwardTranscriber(context, context->sharedBuilder, sink),
        pairBuilderStorage(context),
        autodiffContext(context)
    {
        forwardTranscriber.pairBuilder = &pairBuilderStorage;
        backwardTranscriber.pairBuilder = &pairBuilderStorage;
        backwardTranscriber.fwdDiffTranscriber = &forwardTranscriber;
    }

protected:
    // A transcriber object that handles the main job of 
    // processing instructions while maintaining state.
    //
    ForwardDiffTranscriber forwardTranscriber;

    BackwardDiffTranscriber backwardTranscriber;

    // Diagnostic object from the compile request for
    // error messages.
    DiagnosticSink* sink;

    // Shared context.
    AutoDiffSharedContext* autodiffContext;

    // Builder for dealing with differential pair types.
    DifferentialPairTypeBuilder     pairBuilderStorage;

};

bool processAutodiffCalls(
    IRModule*                           module,
    DiagnosticSink*                     sink,
    IRAutodiffPassOptions const&)
{
    bool modified = false;

    // Create shared context for all auto-diff related passes
    AutoDiffSharedContext autodiffContext(module->getModuleInst());

    // We start by initializing our shared IR building state,
    // since we will re-use that state for any code we
    // generate along the way.
    //
    SharedIRBuilder sharedBuilder;
    sharedBuilder.init(module);
    sharedBuilder.deduplicateAndRebuildGlobalNumberingMap();

    autodiffContext.sharedBuilder = &sharedBuilder;

    AutoDiffPass pass(&autodiffContext, sink);

    modified |= pass.processModule();

    return modified;
}

bool finalizeAutoDiffPass(IRModule* module)
{
    bool modified = false;

    // Create shared context for all auto-diff related passes
    AutoDiffSharedContext autodiffContext(module->getModuleInst());

    SharedIRBuilder sharedBuilder;
    sharedBuilder.init(module);
    sharedBuilder.deduplicateAndRebuildGlobalNumberingMap();

    autodiffContext.sharedBuilder = &sharedBuilder;

    // Replaces IRDifferentialPairType with an auto-generated struct,
    // IRDifferentialPairGetDifferential with 'differential' field access,
    // IRDifferentialPairGetPrimal with 'primal' field access, and
    // IRMakeDifferentialPair with an IRMakeStruct.
    // 
    modified |= processPairTypes(&autodiffContext);

    stripNoDiffTypeAttribute(module);

    // Remove auto-diff related decorations.
    stripAutoDiffDecorations(module);

    return false;
}

}
