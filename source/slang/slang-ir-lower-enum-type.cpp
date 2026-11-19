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

    // Rewrite every `Lsh`/`Rsh` typed by `enumType`, for an enum whose tag type is `bool`, into
    // `bool(int(x) << n)`. Consider `enum Mode : bool { Off, On }` and `m >> 1`: the core module's
    // `__EnumType` shift intrinsics and the synthesized `shl`/`shr` produce `Rsh` on the enum, and
    // the type swap in `processModule` would turn that into `Rsh` on `bool`, which no target can
    // shift. We round-trip through `int` here, the same way `bool.shl`/`bool.shr` do in the core
    // module, so a `bool`-tagged enum shifts like its tag type.
    //
    // The synthesized `shl`/`shr` witnesses of such an enum are body-less `intrinsicOp(Lsh/Rsh)`
    // functions. A direct call is already an `Lsh`/`Rsh` inst here, but a call through a generic
    // `T : ILogical` is only turned into one after specialization, when the enum type is gone, so
    // we give those witnesses an equivalent body instead.
    void lowerBoolTagShifts(IRInst* enumType)
    {
        // A shift's type may be rate-qualified (`@ConstExpr` for `Mode.Off << 1`), in which case
        // its type operand is the rate-qualified type rather than `enumType`, so we search the
        // module instead of walking the uses of `enumType`.
        List<IRInst*> shifts;
        List<IRFunc*> witnesses;
        collectBoolTagShifts(module->getModuleInst(), enumType, shifts, witnesses);
        for (auto shift : shifts)
        {
            IRBuilder builder(shift);
            builder.setInsertBefore(shift);
            auto result = emitBoolTagShift(
                builder,
                shift->getOp(),
                shift->getRate(),
                (IRType*)enumType,
                shift->getOperand(0),
                shift->getOperand(1));
            shift->replaceUsesWith(result);
            shift->removeAndDeallocate();
        }
        for (auto func : witnesses)
        {
            auto intrinsicOp = func->findDecoration<IRIntrinsicOpDecoration>();
            auto funcType = as<IRFuncType>(func->getDataType());
            SLANG_RELEASE_ASSERT(funcType && funcType->getParamCount() == 2);
            IRBuilder builder(func);
            builder.setInsertInto(func);
            builder.emitBlock();
            auto value = builder.emitParam(funcType->getParamType(0));
            auto amount = builder.emitParam(funcType->getParamType(1));
            builder.emitReturn(emitBoolTagShift(
                builder,
                intrinsicOp->getIntrinsicOp(),
                nullptr,
                (IRType*)enumType,
                value,
                amount));
            intrinsicOp->removeAndDeallocate();
        }
    }

    // Collect, under `inst`, the `Lsh`/`Rsh` insts whose data type is `enumType` and the
    // body-less `intrinsicOp(Lsh/Rsh)` functions that return `enumType`.
    void collectBoolTagShifts(
        IRInst* inst,
        IRInst* enumType,
        List<IRInst*>& shifts,
        List<IRFunc*>& witnesses)
    {
        if ((inst->getOp() == kIROp_Lsh || inst->getOp() == kIROp_Rsh) &&
            inst->getDataType() == enumType)
            shifts.add(inst);
        if (auto func = as<IRFunc>(inst); func && !func->getFirstBlock())
        {
            auto intrinsicOp = func->findDecoration<IRIntrinsicOpDecoration>();
            if (intrinsicOp && func->getResultType() == enumType &&
                (intrinsicOp->getIntrinsicOp() == kIROp_Lsh ||
                 intrinsicOp->getIntrinsicOp() == kIROp_Rsh))
                witnesses.add(func);
        }
        for (auto child : inst->getChildren())
            collectBoolTagShifts(child, enumType, shifts, witnesses);
    }

    // Emit `enumType(int(value) OP amount)`, where `OP` is `Lsh` or `Rsh`, with every result
    // qualified by `rate` when it is non-null.
    IRInst* emitBoolTagShift(
        IRBuilder& builder,
        IROp op,
        IRRate* rate,
        IRType* enumType,
        IRInst* value,
        IRInst* amount)
    {
        IRType* intType = builder.getIntType();
        IRType* resultType = enumType;
        if (rate)
        {
            intType = builder.getRateQualifiedType(rate, intType);
            resultType = builder.getRateQualifiedType(rate, resultType);
        }
        auto intValue = builder.emitIntrinsicInst(intType, kIROp_IntCast, 1, &value);
        auto intResult = op == kIROp_Lsh ? builder.emitShl(intType, intValue, amount)
                                         : builder.emitShr(intType, intValue, amount);
        return builder.emitIntrinsicInst(resultType, kIROp_IntCast, 1, &intResult);
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
            {
                canonicalizeBoolTagConstants(key);
                lowerBoolTagShifts(key);
            }
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
