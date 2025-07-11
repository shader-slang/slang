#include "slang-ir-short-string.h"

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

IRInst* getShortStringAsArray(
    IRBuilder& builder,
    IRInst* src,
    IRBasicType* charType,
    bool supportStringLiteral)
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
    if (auto strLit = as<IRStringLit>(src))
    {
        auto sv = strLit->getStringSlice();
        for (UInt i = 0; i < n; ++i)
        {
            IRIntegerValue c = IRIntegerValue(uint8_t(sv[i]));
            chars[i] = builder.getIntValue(charType, c);
        }
    }
    else
    {
        // C/C++ string literals are typed with 'char', but charType might be uint8_t
        // To avoid any error / warning, we need to explicitely cast 'char' to charType
        IRBasicType* strCharType = charType;
        if (supportStringLiteral && srcStrType)
        {
            strCharType = builder.getCharType();
        }
        for (UInt i = 0; i < n; ++i)
        {
            auto getElement = builder.emitGetElement(strCharType, src, i);
            auto casted = builder.emitCast(charType, getElement, false);
            chars[i] = casted;
        }
    }
    auto arrayType = builder.getArrayType(charType, builder.getIntValue(chars.getCount()));
    auto asArray = builder.emitMakeArray(arrayType, chars.getCount(), chars.getBuffer());
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
            auto arrayType = getShortStringArrayType(builder, strLitType);
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
                auto asArray = getShortStringAsArray(builder, strLit);
                inst->replaceUsesWith(asArray);
                inst->removeAndDeallocate();
                changed = true;
            }
        }
        else if (auto getAsArray = as<IRGetShortStringAsArray>(inst))
        {
            auto str = getAsArray->getOperand(0);
            IRInst* replacement = nullptr;
            if (options.targetSupportsStringLiterals)
            {
                IRBuilder builder(module);
                IRBuilderSourceLocRAII srcLocRAII(&builder, inst->sourceLoc);
                builder.setInsertBefore(inst);
                replacement = getShortStringAsArray(
                    builder,
                    str,
                    as<IRBasicType>(as<IRArrayType>(inst->getDataType())->getElementType()),
                    options.targetSupportsStringLiterals);
            }
            else
            {
                replacement = str;
            }
            getAsArray->replaceUsesWith(replacement);
            getAsArray->removeAndDeallocate();
            changed = true;
        }
    }

    void processModule()
    {
        processAllInsts([this](IRInst* inst) { processInst(inst); });
    }
};

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
