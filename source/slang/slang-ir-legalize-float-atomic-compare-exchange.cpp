// slang-ir-legalize-float-atomic-compare-exchange.cpp
#include "slang-ir-legalize-float-atomic-compare-exchange.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-target.h"

namespace Slang
{

struct FloatAtomicCompareExchangeLegalizer
{
    IRModule* m_module;
    TargetRequest* m_targetReq;
    // The code that is emitted, which is GLSL when SPIR-V is emitted through GLSL.
    CodeGenTarget m_target;
    DiagnosticSink* m_sink;

    // The integer-element declaration created for each (buffer parameter, integer type) pair.
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRGlobalParam*> m_aliasParams;

    // The location of an atomic destination inside a buffer, counted in units of the integer
    // type that replaces the floating-point one: `dynamicIndex + constantIndex`, where
    // `dynamicIndex` is null when the whole index is a constant.
    struct BufferLocation
    {
        IRGlobalParam* buffer = nullptr;
        IRPtrTypeBase* elementPtrType = nullptr;
        IRInst* dynamicIndex = nullptr;
        IRIntegerValue constantIndex = 0;
    };

    // Add `index * scale` to `location`.
    void addScaledIndex(
        IRBuilder& builder,
        BufferLocation& location,
        IRInst* index,
        IRIntegerValue scale)
    {
        if (auto lit = as<IRIntLit>(index))
        {
            location.constantIndex += lit->getValue() * scale;
            return;
        }
        auto intType = builder.getIntType();
        IRInst* scaled = index;
        if (scaled->getDataType() != intType)
            scaled = builder.emitCast(intType, scaled);
        if (scale != 1)
            scaled = builder.emitMul(intType, scaled, builder.getIntValue(intType, scale));
        location.dynamicIndex = location.dynamicIndex
                                    ? builder.emitAdd(intType, location.dynamicIndex, scaled)
                                    : scaled;
    }

    IRInst* emitIndex(IRBuilder& builder, BufferLocation const& location)
    {
        auto intType = builder.getIntType();
        auto constantIndex = builder.getIntValue(intType, location.constantIndex);
        if (!location.dynamicIndex)
            return constantIndex;
        if (location.constantIndex == 0)
            return location.dynamicIndex;
        return builder.emitAdd(intType, location.dynamicIndex, constantIndex);
    }

    // The distance in words between consecutive `elementType` values in an array or buffer.
    bool getElementStrideInWords(
        IRTypeLayoutRules* rules,
        IRType* elementType,
        IRIntegerValue wordSize,
        IRIntegerValue& outStride)
    {
        IRSizeAndAlignment elementLayout;
        if (SLANG_FAILED(getSizeAndAlignment(m_targetReq, rules, elementType, &elementLayout)))
            return false;
        auto stride = rules->alignCompositeElement(elementLayout).getStride();
        if (stride % wordSize != 0)
            return false;
        outStride = stride / wordSize;
        return true;
    }

    // Find the buffer and word index that `ptr` addresses. Every step of the address must be a
    // whole number of words, which holds for a 4- or 8-byte scalar under the buffer layouts.
    bool findBufferLocation(
        IRBuilder& builder,
        IRInst* ptr,
        IRIntegerValue wordSize,
        BufferLocation& outLocation)
    {
        switch (ptr->getOp())
        {
        case kIROp_RWStructuredBufferGetElementPtr:
            {
                auto buffer = as<IRGlobalParam>(ptr->getOperand(0));
                if (!buffer)
                    return false;
                auto bufferType = as<IRHLSLStructuredBufferTypeBase>(buffer->getDataType());
                auto ptrType = as<IRPtrTypeBase>(ptr->getDataType());
                if (!bufferType || !ptrType)
                    return false;
                IRIntegerValue stride = 0;
                if (!getElementStrideInWords(
                        getLayoutRules(ptrType),
                        bufferType->getElementType(),
                        wordSize,
                        stride))
                    return false;
                outLocation.buffer = buffer;
                outLocation.elementPtrType = ptrType;
                addScaledIndex(builder, outLocation, ptr->getOperand(1), stride);
                return true;
            }
        case kIROp_FieldAddress:
            {
                auto base = ptr->getOperand(0);
                auto basePtrType = as<IRPtrTypeBase>(base->getDataType());
                if (!basePtrType)
                    return false;
                auto field = findStructField(
                    basePtrType->getValueType(),
                    as<IRStructKey>(ptr->getOperand(1)));
                if (!field)
                    return false;
                IRIntegerValue offset = 0;
                if (SLANG_FAILED(
                        getOffset(m_targetReq, getLayoutRules(basePtrType), field, &offset)))
                    return false;
                if (offset % wordSize != 0)
                    return false;
                if (!findBufferLocation(builder, base, wordSize, outLocation))
                    return false;
                outLocation.constantIndex += offset / wordSize;
                return true;
            }
        case kIROp_GetElementPtr:
            {
                auto base = ptr->getOperand(0);
                auto basePtrType = as<IRPtrTypeBase>(base->getDataType());
                if (!basePtrType)
                    return false;
                auto arrayType = as<IRArrayTypeBase>(basePtrType->getValueType());
                if (!arrayType)
                    return false;
                IRIntegerValue stride = 0;
                if (!getElementStrideInWords(
                        getLayoutRules(basePtrType),
                        arrayType->getElementType(),
                        wordSize,
                        stride))
                    return false;
                if (!findBufferLocation(builder, base, wordSize, outLocation))
                    return false;
                addScaledIndex(builder, outLocation, ptr->getOperand(1), stride);
                return true;
            }
        default:
            return false;
        }
    }

    IRTypeLayoutRules* getLayoutRules(IRPtrTypeBase* ptrType)
    {
        auto dataLayout = ptrType->getDataLayout();
        if (!dataLayout)
            return IRTypeLayoutRules::getStd430();
        auto ruleName =
            getTypeLayoutRuleNameFromOp(dataLayout->getOp(), IRTypeLayoutRuleName::Std430);
        if (!ruleName)
            return IRTypeLayoutRules::getStd430();
        return IRTypeLayoutRules::get(*ruleName);
    }

    // A second declaration of `buffer`, with elements of `intType` and the same binding. Its layout
    // is std430, whatever the layout of `buffer` is: `findBufferLocation` gives a word index, so
    // consecutive elements of the alias have to be one word apart. Under std140 a `uint` array
    // element takes 16 bytes.
    IRGlobalParam* getAliasParam(IRGlobalParam* buffer, IRType* intType)
    {
        KeyValuePair<IRInst*, IRInst*> key(buffer, intType);
        if (auto found = m_aliasParams.tryGetValue(key))
            return *found;

        auto bufferType = cast<IRHLSLStructuredBufferTypeBase>(buffer->getDataType());

        IRBuilder builder(m_module);
        builder.setInsertBefore(buffer);

        List<IRInst*> typeOperands;
        typeOperands.add(intType);
        for (UInt i = 1; i < bufferType->getOperandCount(); i++)
            typeOperands.add(bufferType->getOperand(i));
        if (typeOperands.getCount() > 1)
            typeOperands[1] = getAliasDataLayout(builder);
        auto aliasType =
            builder.getType(bufferType->getOp(), typeOperands.getCount(), typeOperands.getBuffer());

        auto alias = builder.createGlobalParam(aliasType);
        for (auto decoration : buffer->getDecorations())
        {
            switch (decoration->getOp())
            {
            case kIROp_NameHintDecoration:
            case kIROp_LayoutDecoration:
            case kIROp_MemoryQualifierSetDecoration:
            case kIROp_UserTypeNameDecoration:
                cloneDecoration(decoration, alias);
                break;
            default:
                break;
            }
        }
        m_aliasParams.add(key, alias);
        return alias;
    }

    static IRType* getAliasDataLayout(IRBuilder& builder)
    {
        return builder.getType(kIROp_Std430BufferLayoutType);
    }

    // Return an integer-typed pointer to the same memory as `ptr`, or nullptr if there is none.
    IRInst* getIntegerPointer(IRBuilder& builder, IRInst* ptr, IRType* intType)
    {
        auto ptrType = as<IRPtrTypeBase>(ptr->getDataType());
        if (!ptrType)
            return nullptr;

        // A physical storage buffer pointer can be bitcast to another pointee type in SPIR-V.
        if (ptrType->getAddressSpace() == AddressSpace::UserPointer)
        {
            if (!isSPIRV(m_target))
                return nullptr;
            return builder.emitBitCast(builder.getPtrType(intType, ptrType), ptr);
        }

        IRIntegerValue wordSize = getIntTypeInfo(m_targetReq, intType).width / 8;
        BufferLocation location;
        if (!findBufferLocation(builder, ptr, wordSize, location))
            return nullptr;
        auto alias = getAliasParam(location.buffer, intType);
        auto elementPtrType = location.elementPtrType;
        return builder.emitRWStructuredBufferGetElementPtr(
            builder.getPtrType(
                elementPtrType->getOp(),
                intType,
                elementPtrType->getAccessQualifier(),
                elementPtrType->getAddressSpace(),
                getAliasDataLayout(builder)),
            alias,
            emitIndex(builder, location));
    }

    void legalize(IRAtomicCompareExchange* cas)
    {
        auto valueType = cas->getDataType();

        IRBuilder builder(m_module);
        builder.setInsertBefore(cas);

        IRType* intType = nullptr;
        switch (valueType->getOp())
        {
        case kIROp_FloatType:
            intType = builder.getUIntType();
            break;
        case kIROp_DoubleType:
            intType = builder.getUInt64Type();
            break;
        default:
            m_sink->diagnose(Diagnostics::UnsupportedTargetIntrinsic{
                .operation = "16-bit floating point atomic compare-exchange",
                .location = cas->sourceLoc});
            return;
        }

        IRInst* intPtr = getAtomicImageSubscript(cas)
                             ? nullptr
                             : getIntegerPointer(builder, cas->getPtr(), intType);
        if (!intPtr)
        {
            m_sink->diagnose(Diagnostics::UnsupportedTargetIntrinsic{
                .operation = isSPIRV(m_target) ? "floating point atomic compare-exchange outside a "
                                                 "RWStructuredBuffer or a physical storage buffer"
                                               : "floating point atomic compare-exchange outside a "
                                                 "RWStructuredBuffer",
                .location = cas->sourceLoc});
            return;
        }

        // Operands after the comparand and the new value are the memory orders.
        List<IRInst*> args;
        args.add(intPtr);
        args.add(builder.emitBitCast(intType, cas->getExpected()));
        args.add(builder.emitBitCast(intType, cas->getDesired()));
        for (UInt i = 3; i < cas->getOperandCount(); i++)
            args.add(cas->getOperand(i));
        auto intCas = builder.emitIntrinsicInst(
            intType,
            kIROp_AtomicCompareExchange,
            args.getCount(),
            args.getBuffer());
        intCas->sourceLoc = cas->sourceLoc;
        auto result = builder.emitBitCast(valueType, intCas);
        cas->replaceUsesWith(result);
        cas->removeAndDeallocate();
    }

    void processModule()
    {
        List<IRAtomicCompareExchange*> worklist;
        for (auto globalInst : m_module->getGlobalInsts())
        {
            auto func = as<IRGlobalValueWithCode>(globalInst);
            if (!func)
                continue;
            for (auto block : func->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    if (auto cas = as<IRAtomicCompareExchange>(inst))
                    {
                        if (isFloatingType(cas->getDataType()))
                            worklist.add(cas);
                    }
                }
            }
        }
        for (auto cas : worklist)
            legalize(cas);
    }
};

void legalizeFloatAtomicCompareExchange(
    IRModule* module,
    TargetRequest* targetReq,
    CodeGenTarget target,
    DiagnosticSink* sink)
{
    FloatAtomicCompareExchangeLegalizer legalizer{module, targetReq, target, sink};
    legalizer.processModule();
}

} // namespace Slang
