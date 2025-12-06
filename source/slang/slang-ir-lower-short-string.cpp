#include "slang-ir-lower-short-string.h"

#include "slang-ir-inst-pass-base.h"

namespace Slang
{

IRArrayType* getShortStringArrayType(
    IRBuilder& builder,
    IRShortStringType* strLitType,
    IRBasicType* charType)
{
    if (!charType)
    {
        charType = builder.getUInt8Type();
    }
    return builder.getArrayType(charType, strLitType->getLength());
}

IRArrayType* getShortStringPackedArray32Type(
    IRBuilder& builder,
    IRShortStringType* strLitType,
    IRInst* count)
{
    auto l = strLitType->getLength();
    if (!count)
    {
        if (auto intLit = as<IRIntLit>(l))
        {
            count = builder.getIntValue((intLit->getValue() + 3) / 4);
        }
        else
        {
            count = builder.emitShr(
                builder.getIntType(),
                builder.emitAdd(builder.getIntType(), l, builder.getIntValue(3)),
                builder.getIntValue(2));
        }
    }
    IRArrayType* res = builder.getArrayType(builder.getUIntType(), count);
    return res;
}

IRInst* getShortStringAsArray(IRBuilder& builder, IRStringLit* src, IRBasicType* charType)
{
    UInt n = 0;
    auto type = src->getDataType();
    auto srcStrType = as<IRShortStringType>(type);
    if (srcStrType)
    {
        n = static_cast<UInt>(as<IRIntLit>(srcStrType->getLength())->value.intVal);
    }
    else if (auto arrayType = as<IRArrayType>(type))
    {
        n = static_cast<UInt>(as<IRIntLit>(arrayType->getElementCount())->value.intVal);
    }
    else
    {
        SLANG_UNEXPECTED("Type not handled!");
    }
    List<IRInst*> chars;
    chars.setCount(n);
    if (!charType)
    {
        charType = builder.getUInt8Type();
    }
    auto sv = src->getStringSlice();
    for (UInt i = 0; i < n; ++i)
    {
        IRIntegerValue c = IRIntegerValue(uint8_t(sv[i]));
        chars[i] = builder.getIntValue(charType, c);
    }
    auto arrayType = builder.getArrayType(charType, builder.getIntValue(chars.getCount()));
    auto asArray = builder.emitMakeArray(arrayType, chars.getCount(), chars.getBuffer());
    return asArray;
}

IRInst* getShortStringAsPackedArray32(IRBuilder& builder, IRStringLit* src)
{
    UInt len = 0;
    auto type = src->getDataType();
    auto srcStrType = as<IRShortStringType>(type);
    auto sv = src->getStringSlice();
    if (srcStrType)
    {
        len = static_cast<UInt>(as<IRIntLit>(srcStrType->getLength())->value.intVal);
    }
    else
    {
        len = static_cast<UInt>(sv.getLength());
    }
    UInt capacity = (len + 3) / 4;
    List<IRInst*> chunks;
    chunks.setCount(capacity);
    IRBasicType* chunkType = builder.getUIntType();
    for (UInt i = 0; i < capacity; ++i)
    {
        uint32_t chunk = 0;
        for (uint32_t j = 0; j < 4; ++j)
        {
            if ((4 * i + j) < static_cast<UInt>(sv.getLength()))
            {
                chunk |= (uint32_t(uint8_t(sv[4 * i + j])) << (8 * j));
            }
        }
        chunks[i] = builder.getIntValue(chunkType, chunk);
    }
    auto arrayType = builder.getArrayType(chunkType, builder.getIntValue(capacity));
    auto asArray = builder.emitMakeArray(arrayType, capacity, chunks.getBuffer());
    return asArray;
}

struct ShortStringLoweringPass : InstPassBase
{
    ShortStringsOptions options;

    ShortStringLoweringPass(IRModule* irModule, ShortStringsOptions const& options)
        : InstPassBase(irModule), options(options)
    {
    }

    bool changed = false;

    void processInst(IRInst* inst)
    {
        if (auto strLitType = as<IRShortStringType>(inst);
            !options.targetSupportsStringLiterals && strLitType)
        {
            IRBuilder builder(module);
            IRBuilderSourceLocRAII srcLocRAII(&builder, inst->sourceLoc);
            builder.setInsertBefore(inst);
            IRArrayType* arrayType;
            if (options.targetSupports8BitsIntegrals)
            {
                arrayType = getShortStringArrayType(builder, strLitType);
            }
            else
            {
                arrayType = getShortStringPackedArray32Type(builder, strLitType);
            }
            inst->replaceUsesWith(arrayType);
            inst->removeAndDeallocate();
            changed = true;
        }
        else if (auto strLit = as<IRStringLit>(inst);
                 !options.targetSupportsStringLiterals && strLit)
        {
            if (as<IRShortStringType>(strLit->getDataType()) ||
                as<IRArrayType>(strLit->getDataType()))
            {
                IRBuilder builder(module);
                IRBuilderSourceLocRAII srcLocRAII(&builder, inst->sourceLoc);
                builder.setInsertBefore(inst);
                IRInst* asArray = nullptr;
                if (options.targetSupports8BitsIntegrals)
                {
                    asArray = getShortStringAsArray(builder, strLit);
                }
                else
                {
                    asArray = getShortStringAsPackedArray32(builder, strLit);
                }
                inst->replaceUsesWith(asArray);
                inst->removeAndDeallocate();
                changed = true;
            }
        }
        else if (auto getChar = as<IRGetCharFromString>(inst))
        {
            IRBuilder builder(module);
            IRBuilderSourceLocRAII srcLocRAII(&builder, inst->sourceLoc);
            builder.setInsertBefore(inst);
            IRInst* substitute = nullptr;
            if (options.targetSupportsStringLiterals || options.targetSupports8BitsIntegrals)
            {
                // The ShortString is/will be lowered to a string literal (const char[N+1]), or an
                // Array<uint8_t, N> So we need to cast the result of getElement
                IRBasicType* getElementType = options.targetSupportsStringLiterals
                                                  ? as<IRBasicType>(builder.getCharType())
                                                  : as<IRBasicType>(builder.getUInt8Type());
                IRInst* getElement = builder.emitElementExtract(
                    getElementType,
                    getChar->getBase(),
                    getChar->getIndex());
                if (options.targetSupportsStringLiterals)
                {
                    // In C++, char is usualy signed, so a simple cast uint32_t(char(c)) would lead
                    // to a sign bit extension We prefer to keep char values in [0, 255], so we need
                    // a second cast: uint32_t(uint8_t(char(c)))
                    getElement = builder.emitCast(builder.getUInt8Type(), getElement);
                }
                auto casted = builder.emitCast(inst->getDataType(), getElement);
                substitute = casted;
            }
            else
            {
                auto charIndex = getChar->getIndex();
                charIndex = builder.emitCast(builder.getUIntType(), charIndex);
                // The short string is stored as a packed u32 array
                // str[i] := bitfieldExtract(str_u32_packed_array[i / 4], (i % 4) * 8, 8);
                // i / 4 := i >> 2
                auto u32Index = builder.emitShr(
                    builder.getUIntType(),
                    charIndex,
                    builder.getIntValue(builder.getUIntType(), 2));
                // i % 4 != i & 0b11
                auto indexInU32 = builder.emitBitAnd(
                    builder.getUIntType(),
                    charIndex,
                    builder.getIntValue(builder.getUIntType(), 0b11));
                // (i % 4) * 8 := (i % 4) << 3
                auto bitIndex = builder.emitShl(
                    builder.getUIntType(),
                    indexInU32,
                    builder.getIntValue(builder.getUIntType(), 3));
                auto getElement =
                    builder.emitElementExtract(builder.getUIntType(), getChar->getBase(), u32Index);
                auto extractedChar = builder.emitBitfieldExtract(
                    inst->getDataType(),
                    getElement,
                    bitIndex,
                    builder.getIntValue(builder.getUIntType(), 8));
                substitute = extractedChar;
            }
            inst->replaceUsesWith(substitute);
            inst->removeAndDeallocate();
            changed = true;
        }
    }

    void processModule()
    {
        processAllInsts([this](IRInst* inst) { processInst(inst); });
    }
};

ShortStringsOptions::ShortStringsOptions(CodeGenTarget target)
{
    switch (target)
    {
    case CodeGenTarget::CSource:
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::PyTorchCppBinding:
    case CodeGenTarget::HostCPPSource:
    case CodeGenTarget::HostExecutable:
    case CodeGenTarget::HostSharedLibrary:
    case CodeGenTarget::ShaderSharedLibrary:
    case CodeGenTarget::ShaderHostCallable:
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::PTX:
    case CodeGenTarget::ObjectCode:
    case CodeGenTarget::HostHostCallable:
        targetSupportsStringLiterals = true;
        break;
    default:
        targetSupportsStringLiterals = false;
        break;
    }
    switch (target)
    {
    case CodeGenTarget::HLSL:
    case CodeGenTarget::DXBytecode:
    case CodeGenTarget::DXBytecodeAssembly:
    case CodeGenTarget::DXIL:
    case CodeGenTarget::DXILAssembly:
    case CodeGenTarget::WGSL:
        targetSupports8BitsIntegrals = false;
        break;
    default:
        targetSupports8BitsIntegrals = true;
        break;
    }
}

bool lowerShortStringReturnChanged(
    TargetProgram* target,
    IRModule* module,
    ShortStringsOptions options)
{
    SLANG_UNUSED(target);
    bool res = false;
    ShortStringLoweringPass pass(module, options);
    pass.processModule();
    res |= pass.changed;
    return res;
}
} // namespace Slang
