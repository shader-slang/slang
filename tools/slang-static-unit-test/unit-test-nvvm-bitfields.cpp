// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "nvvm-static-test-context.h"
#include "slang-unit-test/unit-test-nvvm-support.h"
#include "slang/slang-emit-nvvm.h"
#include "slang/slang-ir-nvvm-legalize.h"

using namespace Slang;

// Reject malformed canonical instructions at legalization, before constructing a provider module.
// Public source arguments normally coerce to UInt32; direct IR tests also protect those signatures.
SLANG_UNIT_TEST(nvvmBitfieldLegalizationRejectsInvalidSignatures)
{
    enum class Invalid
    {
        Float,
        Bool,
        Array,
        OneLaneVector,
        FiveLaneVector,
        SignedOffset,
        VectorCount,
        MismatchedValue,
        MismatchedInsert,
        OperandCount,
        NullOperand,
    };
    for (auto invalid :
         {Invalid::Float,
          Invalid::Bool,
          Invalid::Array,
          Invalid::OneLaneVector,
          Invalid::FiveLaneVector,
          Invalid::SignedOffset,
          Invalid::VectorCount,
          Invalid::MismatchedValue,
          Invalid::MismatchedInsert,
          Invalid::OperandCount,
          Invalid::NullOperand})
    {
        _resetDirectNVVMFakes();
        NVVMStaticTestContext context(unitTestContext);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        IRType* type = builder.getUIntType();
        switch (invalid)
        {
        case Invalid::Float:
            type = builder.getFloatType();
            break;
        case Invalid::Bool:
            type = builder.getBoolType();
            break;
        case Invalid::Array:
            type = builder.getArrayTypeBase(
                kIROp_ArrayType,
                type,
                builder.getIntValue(builder.getIntType(), 2));
            break;
        case Invalid::OneLaneVector:
            type = builder.getVectorType(type, 1);
            break;
        case Invalid::FiveLaneVector:
            type = builder.getVectorType(type, 5);
            break;
        default:
            break;
        }
        IRType* offsetType = builder.getUIntType();
        if (invalid == Invalid::SignedOffset)
            offsetType = builder.getIntType();
        IRType* countType = builder.getUIntType();
        if (invalid == Invalid::VectorCount)
            countType = builder.getVectorType(builder.getUIntType(), 2);
        IRType* valueType = invalid == Invalid::MismatchedValue ? builder.getIntType() : type;
        IRType* parameters[] = {valueType, offsetType, countType, builder.getIntType()};
        auto function = builder.createFunc();
        function->setFullType(builder.getFuncType(4, parameters, type));
        builder.addKeepAliveDecoration(function);
        builder.setInsertInto(function);
        builder.emitBlock();
        auto value = builder.emitParam(valueType);
        auto offset = builder.emitParam(offsetType);
        auto count = builder.emitParam(countType);
        auto inserted = builder.emitParam(builder.getIntType());
        IRInst* operation = nullptr;
        if (invalid == Invalid::MismatchedInsert)
            operation = builder.emitBitfieldInsert(type, value, inserted, offset, count);
        else
        {
            IRInst* operands[] = {invalid == Invalid::NullOperand ? nullptr : value, offset, count};
            operation = builder.emitIntrinsicInst(
                type,
                kIROp_BitfieldExtract,
                invalid == Invalid::OperandCount ? 2 : 3,
                operands);
        }
        builder.emitReturn(operation);
        LinkedIR linked = {};
        linked.module = module;
        SLANG_CHECK(SLANG_FAILED(legalizeIRForNVVM(&context.codeGen, linked)));
        SLANG_CHECK(context.sink.outputBuffer.indexOf("bitfield signature") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

// Canonical pure module expressions must move to their consuming block after bitfield expansion.
SLANG_UNIT_TEST(nvvmBitfieldModuleExpressionsUseOrdinaryPlacement)
{
    _resetDirectNVVMFakes();
    NVVMStaticTestContext context(unitTestContext);
    auto module = IRModule::create(context.env.getSessionImpl());
    IRBuilder builder(module);
    builder.setInsertInto(module);
    auto type = builder.getUIntType();
    auto zero = builder.getIntValue(type, 0);
    auto width = builder.getIntValue(type, 32);
    auto value = builder.getIntValue(type, 0xa5);
    auto inserted = builder.emitBitfieldInsert(type, value, value, zero, width);
    auto extracted = builder.emitBitfieldExtract(type, inserted, zero, width);
    auto pointerType = builder.getPtrType(
        type,
        AccessQualifier::ReadWrite,
        AddressSpace::UserPointer,
        builder.getDefaultBufferLayoutType());
    IRType* parameters[] = {pointerType};
    auto entry = builder.createFunc();
    entry->setFullType(builder.getFuncType(1, parameters, builder.getVoidType()));
    builder.addEntryPointDecoration(
        entry,
        Profile(Stage::Compute),
        toSlice("computeMain"),
        toSlice("test"));
    builder.addKeepAliveDecoration(entry);
    builder.setInsertInto(entry);
    auto block = builder.emitBlock();
    auto output = builder.emitParam(pointerType);
    auto store = cast<IRStore>(builder.emitStore(output, extracted));
    builder.emitReturn();
    LinkedIR linked = {};
    linked.module = module;
    linked.entryPoints.add(entry);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(legalizeIRForNVVM(&context.codeGen, linked)));
    SLANG_CHECK(store->getVal()->getParent() == block);
    for (auto inst : block->getOrdinaryInsts())
    {
        SLANG_CHECK(inst->getOp() != kIROp_BitfieldExtract);
        SLANG_CHECK(inst->getOp() != kIROp_BitfieldInsert);
    }
    NVVMOperationRequirements requirements;
    SLANG_CHECK(SLANG_SUCCEEDED(validateNVVMSupportedIR(&context.codeGen, linked, requirements)));
    SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
}
