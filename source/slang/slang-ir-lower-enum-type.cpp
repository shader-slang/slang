// slang-ir-lower-enum-type.cpp

#include "slang-ir-lower-enum-type.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct EnumTypeLoweringContext
{
    IRModule* module;
    DiagnosticSink* sink;

    InstWorkList workList;
    InstHashSet workListSet;

    IRGeneric* genericOptionalStructType = nullptr;
    IRStructKey* valueKey = nullptr;
    IRStructKey* hasValueKey = nullptr;

    EnumTypeLoweringContext(IRModule* inModule)
        : module(inModule), workList(inModule), workListSet(inModule)
    {
    }

    struct LoweredEnumTypeInfo : public RefObject
    {
        IRType* enumType = nullptr;
        IRType* loweredType = nullptr;
    };
    Dictionary<IRInst*, RefPtr<LoweredEnumTypeInfo>> loweredEnumTypes;

    void addToWorkList(IRInst* inst)
    {
        if (workListSet.contains(inst))
            return;

        workList.add(inst);
        workListSet.add(inst);
    }

    LoweredEnumTypeInfo* getLoweredEnumType(IRInst* type)
    {
        if (auto loweredInfo = loweredEnumTypes.tryGetValue(type))
            return loweredInfo->Ptr();

        if (!type)
            return nullptr;

        if (auto attributedType = as<IRAttributedType>(type))
        {
            // Recursively get the lowered enum type for the base type
            auto baseLoweredInfo = getLoweredEnumType(type->getOperand(0));
            if (!baseLoweredInfo)
                return nullptr; // Base type is not an enum, so this isn't an enum type either

            IRBuilder builder(module);

            List<IRAttr*> attrs;
            for (auto attr : attributedType->getAllAttrs())
                attrs.add(attr);

            RefPtr<LoweredEnumTypeInfo> info = new LoweredEnumTypeInfo();
            info->enumType = (IRType*)type;
            info->loweredType = builder.getAttributedType(baseLoweredInfo->loweredType, attrs);
            loweredEnumTypes[type] = info;
            return info.Ptr();
        }

        if (type->getOp() != kIROp_EnumType)
            return nullptr;

        RefPtr<LoweredEnumTypeInfo> info = new LoweredEnumTypeInfo();
        auto enumType = cast<IREnumType>(type);
        auto valueType = enumType->getTagType();
        info->enumType = (IRType*)type;
        info->loweredType = valueType;
        loweredEnumTypes[type] = info;
        return info.Ptr();
    }

    void processEnumType(IREnumType* inst)
    {
        auto loweredEnumTypeInfo = getLoweredEnumType(inst);
        SLANG_ASSERT(loweredEnumTypeInfo);
        SLANG_UNUSED(loweredEnumTypeInfo);
    }

    void processEnumCast(IRInst* inst)
    {
        IRBuilder builderStorage(module);
        auto builder = &builderStorage;
        builder->setInsertBefore(inst);

        auto value = inst->getOperand(0);
        if (auto enumType = getLoweredEnumType(value->getDataType()))
        {
            auto rate = value->getRate();
            auto type = enumType->loweredType;
            if (rate)
            {
                type = builder->getRateQualifiedType(rate, type);
            }

            value->setFullType(type);
        }

        auto type = inst->getDataType();
        if (auto enumType = getLoweredEnumType(type))
        { // Cast was into enum, so use tag type instead.
            type = enumType->loweredType;
        }

        auto cast = builder->emitCast(type, value);

        inst->replaceUsesWith(cast);
        inst->removeAndDeallocate();
    }

    // Rewrite every `IRIntLit` typed by `enumType` into a canonical `IRBoolLit`, for an enum
    // whose tag type is `bool`. Such a constant is built (e.g. a switch case label in
    // `lowerSwitchCases`) while its type is still the opaque `IREnumType`, so the bool
    // canonicalization in `IRBuilder::getIntValue` is bypassed; the type-operand swap in
    // `processModule` alone would then leave an `IRIntLit` of `bool` type -- a second
    // representation of a bool constant that breaks `IRBoolLit` identity/dedup and has no bool
    // arm in the C-like literal emitter. Canonicalizing at this producer keeps one form.
    void canonicalizeBoolTagConstants(IRInst* enumType)
    {
        IRBuilder builder(module);

        // `replaceUsesWith` below rewrites the enumerator constants' users; collect the
        // constants first so we are not mutating a use list we are still walking.
        List<IRIntLit*> enumConstants;
        for (auto use = enumType->firstUse; use; use = use->nextUse)
        {
            auto lit = as<IRIntLit>(use->getUser());
            if (lit && lit->getFullType() == enumType)
                enumConstants.add(lit);
        }

        // Remove each replaced literal so no `IRIntLit` of `bool` type survives in the module
        // (or its constant-dedup map / serialized form) alongside the canonical `IRBoolLit`.
        for (auto lit : enumConstants)
        {
            lit->replaceUsesWith(builder.getBoolValue(lit->getValue() != 0));
            lit->removeAndDeallocate();
        }
    }

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_EnumType:
            processEnumType((IREnumType*)inst);
            break;
        case kIROp_CastEnumToInt:
        case kIROp_CastIntToEnum:
        case kIROp_EnumCast:
            processEnumCast(inst);
            break;
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

        // Replace all enum types with their lowered equivalent types.
        for (const auto& [key, value] : loweredEnumTypes)
        {
            // For a `bool`-tagged enum, first rewrite its enumerator constants to canonical
            // `IRBoolLit`; the type-operand swap alone would leave a non-canonical `IRIntLit`
            // of `bool` type (see canonicalizeBoolTagConstants).
            if (value->loweredType->getOp() == kIROp_BoolType)
                canonicalizeBoolTagConstants(key);
            key->replaceUsesWith(value->loweredType);
        }
    }
};

void lowerEnumType(IRModule* module, DiagnosticSink* sink)
{
    EnumTypeLoweringContext context(module);
    context.sink = sink;
    context.processModule();
}
} // namespace Slang
