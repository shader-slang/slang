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
        charType = builder.getUIntType();
    }
    return builder.getArrayType(charType, strLitType->getLength());
}

IRInst* getShortStringAsArray(IRBuilder& builder, IRStringLit* strLit, IRBasicType* charType)
{
    auto sv = strLit->getStringSlice();
    List<IRInst*> chars;
    chars.reserve(sv.getLength());
    if (!charType)
    {
        charType = builder.getUIntType();
    }
    for (uint32_t i = 0; i < sv.getLength(); ++i)
    {
        // TODO check encoding
        uint32_t c = uint8_t(sv[i]);
        chars.add(builder.getIntValue(charType, c));
    }
    auto arrayType = builder.getArrayType(charType, builder.getIntValue(chars.getCount()));
    auto asArray = builder.emitMakeArray(arrayType, chars.getCount(), chars.getBuffer());
    return asArray;
}

struct ShortStringReplacementPass : InstPassBase
{
    ShortStringReplacementPass(IRModule* irModule)
        : InstPassBase(irModule)
    {
    }

    bool changed = false;

    void processInst(IRInst* inst)
    {
        if (auto strLitType = as<IRShortStringType>(inst))
        {
            IRBuilder builder(module);
            IRBuilderSourceLocRAII srcLocRAII(&builder, inst->sourceLoc);
            builder.setInsertBefore(inst);
            auto arrayType = getShortStringArrayType(builder, strLitType);
            inst->replaceUsesWith(arrayType);
            inst->removeAndDeallocate();
            changed = true;
        }
        else if (auto strLit = as<IRStringLit>(inst))
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
    }

    void processModule()
    {
        processAllInsts([this](IRInst* inst) { processInst(inst); });
    }
};

bool replaceShortStringReturnChanged(
    TargetProgram* target,
    IRModule* module,
    ShortStringsOptions options)
{
    SLANG_UNUSED(target);
    bool res = false;
    if (options.replaceShortStringsWithArray)
    {
        ShortStringReplacementPass pass(module);
        pass.processModule();
        res |= pass.changed;
    }
    return res;
}
} // namespace Slang
