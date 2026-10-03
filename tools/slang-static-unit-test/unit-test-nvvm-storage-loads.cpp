// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "nvvm-static-test-context.h"
#include "slang/slang-ir-lower-buffer-element-type.h"

using namespace Slang;

// Consider a helper that captures a record, changes the original, and returns the capture:
//
//     Payload capture(inout Payload source)
//     {
//         let saved = source;
//         source.tag = 91;
//         return saved;
//     }
//
// Compact CUDA storage must be unpacked at the original read, so saved retains the old tag.
// Ordinary loads can unpack directly from the source address. Attributed loads must keep their
// physical memory operation; unpacking them into ordinary leaf loads would discard its contract.
SLANG_UNIT_TEST(nvvmStorageLoadsPreserveSnapshotAndAttributes)
{
    enum class LoadKind
    {
        Ordinary,
        Aligned,
        Scoped,
        ScopedAligned,
    };
    for (auto kind :
         {LoadKind::Ordinary, LoadKind::Aligned, LoadKind::Scoped, LoadKind::ScopedAligned})
    {
        NVVMStaticTestContext context(unitTestContext);
        context.targetProgram->getOptionSet().set(
            CompilerOptionName::EmitCUDAMethod,
            SLANG_EMIT_CUDA_VIA_NVVM);
        auto module = IRModule::create(context.env.getSessionImpl());
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto vectorType = builder.getVectorType(builder.getFloatType(), 3);
        auto arrayType = builder.getArrayTypeBase(
            kIROp_ArrayType,
            vectorType,
            builder.getIntValue(builder.getIntType(), 2));
        auto recordType = builder.createStructType();
        builder.createStructField(recordType, builder.createStructKey(), vectorType);
        builder.createStructField(recordType, builder.createStructKey(), arrayType);
        auto tagField =
            builder.createStructField(recordType, builder.createStructKey(), builder.getUIntType());
        auto pointerType = builder.getPtrType(recordType);
        IRType* parameters[] = {pointerType};
        auto function = builder.createFunc();
        function->setFullType(builder.getFuncType(1, parameters, recordType));
        builder.addKeepAliveDecoration(function);
        builder.setInsertInto(function);
        auto block = builder.emitBlock();
        auto source = builder.emitParam(pointerType);

        ShortList<IRInst*> attributes;
        if (kind == LoadKind::Aligned || kind == LoadKind::ScopedAligned)
        {
            IRInst* alignment = builder.getIntValue(builder.getIntType(), 4);
            attributes.add(builder.getAttr(kIROp_AlignedAttr, 1, &alignment));
        }
        if (kind == LoadKind::Scoped || kind == LoadKind::ScopedAligned)
        {
            IRInst* scope = builder.getIntValue(builder.getIntType(), int(MemoryScope::Device));
            attributes.add(builder.getAttr(kIROp_MemoryScopeAttr, 1, &scope));
        }
        auto saved = builder.emitLoad(recordType, source, attributes.getArrayView().arrayView);
        auto tagAddress = builder.emitFieldAddress(source, tagField->getKey());
        auto mutationValue = builder.getIntValue(builder.getUIntType(), 91);
        builder.emitStore(tagAddress, mutationValue);
        auto returnInst = builder.emitReturn(saved);

        BufferElementTypeLoweringOptions options;
        options.loweringPolicyKind = BufferElementTypeLoweringPolicyKind::NVVM;
        lowerBufferElementTypeToStorageType(module, context.targetProgram, &context.sink, options);
        SLANG_CHECK_ABORT(context.sink.getErrorCount() == 0);
        auto storageType = cast<IRPtrTypeBase>(source->getDataType())->getValueType();
        SLANG_CHECK_ABORT(storageType != recordType);

        // Inspect the shared pass before inlining or value propagation can hide its decision.
        auto unpack = as<IRCall>(returnInst->getOperand(0));
        SLANG_CHECK_ABORT(unpack);
        SLANG_CHECK(unpack->getDataType() == recordType);
        SLANG_CHECK_ABORT(unpack->getArgCount() == 1);
        bool sawUnpack = false;
        bool sawMutation = false;
        Index storageLoadCount = 0;
        IRLoad* storageLoad = nullptr;
        for (auto inst : block->getOrdinaryInsts())
        {
            if (inst == unpack)
            {
                if (kind == LoadKind::Ordinary)
                    SLANG_CHECK(!sawMutation);
                sawUnpack = true;
            }
            if (auto store = as<IRStore>(inst))
            {
                if (store->getVal() == mutationValue)
                {
                    if (kind == LoadKind::Ordinary)
                        SLANG_CHECK(sawUnpack);
                    sawMutation = true;
                }
            }
            if (auto load = as<IRLoad>(inst))
            {
                if (load->getDataType() == storageType)
                {
                    SLANG_CHECK(!sawMutation);
                    SLANG_CHECK(load->getPtr() == source);
                    storageLoad = load;
                    ++storageLoadCount;
                }
            }
        }
        SLANG_CHECK(sawUnpack && sawMutation);
        if (kind == LoadKind::Ordinary)
        {
            SLANG_CHECK(storageLoadCount == 0);
            SLANG_CHECK(unpack->getArg(0) == source);
        }
        else
        {
            // Scoped aggregate loads are not newly admitted by this test. Their original
            // operation must survive for downstream validation to enforce that boundary.
            SLANG_CHECK_ABORT(storageLoadCount == 1);
            SLANG_CHECK(storageLoad->getAllAttrs().getCount() == attributes.getCount());
            for (auto attribute : attributes)
            {
                bool found = false;
                for (auto preserved : storageLoad->getAllAttrs())
                    found |= preserved == attribute;
                SLANG_CHECK(found);
            }
            SLANG_CHECK(unpack->getArg(0) != source);
            SLANG_CHECK(as<IRVar>(unpack->getArg(0)) != nullptr);
        }
    }
}
