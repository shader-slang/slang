#include "slang-ir-hgbc-owned-erasure.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-rich-diagnostics.h"
#include "slang-target-program.h"

namespace Slang
{
bool usesHGBCOwnedErasure(TargetProgram* targetProgram)
{
    return targetProgram->getTargetReq()->getTarget() == CodeGenTarget::HGBC;
}

bool tryGetHGBCOpaqueTypeOpcode(
    IRInst* inst,
    CapabilitySet const& targetCaps,
    HGBCOpcode& outOpcode)
{
    UnownedStringSlice definition;
    IRInst* intrinsic = nullptr;
    if (!findTargetIntrinsicDefinition(inst, targetCaps, definition, intrinsic))
        return false;
    return findHGBCOpcodeByName(definition.begin(), definition.getLength(), outOpcode) &&
           hasHGBCOpcodeFlag(outOpcode, HGBCOpcodeFlags::OpaqueType);
}

IRType* getHGBCBoxType(IRBuilder& builder)
{
    return cast<IRType>(builder.emitIntrinsicInst(nullptr, kIROp_HGVMBoxType, 0, nullptr));
}

IRInst* emitHGBCBox(IRBuilder& builder, IRInst* value)
{
    auto boxType = getHGBCBoxType(builder);
    if (value->getDataType() == boxType)
        return value;
    // Typeflow's `none` marker becomes Void in untagged-union lowering. It has no payload
    // even before that pass, so creating a box here would later become an invalid Box<Void>.
    if (value->getDataType()->getOp() == kIROp_VoidType ||
        value->getDataType()->getOp() == kIROp_NoneTypeElement)
        return builder.emitDefaultConstruct(boxType);
    return builder.emitIntrinsicInst(boxType, kIROp_HGVMBox, 1, &value);
}

IRInst* emitHGBCPayloadConversion(IRBuilder& builder, IRType* type, IRInst* value)
{
    if (type == value->getDataType())
        return value;
    if (type->getOp() == kIROp_HGVMBoxType)
        return emitHGBCBox(builder, value);
    SLANG_ASSERT(value->getDataType()->getOp() == kIROp_HGVMBoxType);
    return builder.emitIntrinsicInst(type, kIROp_HGVMUnbox, 1, &value);
}

void lowerHGBCOwnedPayloads(IRModule* module)
{
    InstPassBase context(module);
    context.processAllInsts(
        [&](IRInst* inst)
        {
            switch (inst->getOp())
            {
            case kIROp_PackAnyValue:
            case kIROp_UnpackAnyValue:
            case kIROp_Reinterpret:
                break;
            default:
                return;
            }
            auto value = inst->getOperand(0);
            if (inst->getDataType()->getOp() != kIROp_HGVMBoxType &&
                value->getDataType()->getOp() != kIROp_HGVMBoxType)
                return;
            IRBuilder builder(inst);
            builder.setInsertBefore(inst);
            inst->replaceUsesWith(emitHGBCPayloadConversion(builder, inst->getDataType(), value));
            inst->removeAndDeallocate();
        });
}
/// Classifies the ordinary values that can survive packing into integer words. A pointer or
/// opaque host handle is excluded because its ownership cannot be encoded in those words.
static bool isOrdinaryBytePayload(IRType* type, CapabilitySet const& targetCaps)
{
    if (as<IRBasicType>(type) || as<IRPackedFloatType>(type) || as<IRAnyValueType>(type))
        return true;
    if (auto structure = as<IRStructType>(type))
    {
        // A target annotation replaces the declared field layout. For example, a ray pipeline's
        // uint field describes its GPU binding, while its HGBC representation owns a registry.
        HGBCOpcode opcode;
        if (tryGetHGBCOpaqueTypeOpcode(structure, targetCaps, opcode))
            return false;
        for (auto field : structure->getFields())
            if (!isOrdinaryBytePayload(field->getFieldType(), targetCaps))
                return false;
        return true;
    }
    if (auto array = as<IRArrayType>(type))
        return isOrdinaryBytePayload(array->getElementType(), targetCaps);
    if (auto vector = as<IRVectorType>(type))
        return isOrdinaryBytePayload(vector->getElementType(), targetCaps);
    if (auto matrix = as<IRMatrixType>(type))
        return isOrdinaryBytePayload(matrix->getElementType(), targetCaps);
    if (auto enumeration = as<IREnumType>(type))
        return isOrdinaryBytePayload(enumeration->getTagType(), targetCaps);
    return false;
}

void validateHGBCOrdinaryBytePacking(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink)
{
    InstPassBase context(module);
    context.processAllInsts(
        [&](IRInst* inst)
        {
            IRType* concreteType = nullptr;
            if (inst->getOp() == kIROp_PackAnyValue)
                concreteType = inst->getOperand(0)->getDataType();
            else if (inst->getOp() == kIROp_UnpackAnyValue)
                concreteType = inst->getDataType();
            // Specialization can turn a symbolic pack into an identity. The ordinary
            // marshalling pass removes it; it does not erase ownership into bytes.
            if (concreteType && inst->getDataType() == inst->getOperand(0)->getDataType())
                return;
            if (concreteType && !isOrdinaryBytePayload(
                                    concreteType,
                                    targetProgram->getTargetReq()->getTargetCaps()))
                sink->diagnose(Diagnostics::TypeCannotBePackedIntoAnyValue{
                    .type = concreteType,
                    .location = inst->sourceLoc});
        });
}
} // namespace Slang
