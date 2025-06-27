#include "slang-ir-string-literals.h"

#include "slang-ir-inst-pass-base.h"

namespace Slang
{

IRArrayType* getStringLiteralArrayType(IRBuilder& builder, IRStringLiteralType* strLitType)
{
    return builder.getArrayType(builder.getUIntType(), strLitType->getLength());
}

IRInst* getStringLiteralAsArray(IRBuilder& builder, IRStringLit* strLit)
{
    auto sv = strLit->getStringSlice();
    List<IRInst*> chars;
    chars.reserve(sv.getLength());
    for (uint32_t i = 0; i < sv.getLength(); ++i)
    {
        // TODO check encoding
        uint32_t c = uint8_t(sv[i]);
        chars.add(builder.getIntValue(builder.getUIntType(), c));
    }
    auto arrayType =
        builder.getArrayType(builder.getUIntType(), builder.getIntValue(chars.getCount()));
    auto asArray = builder.emitMakeArray(arrayType, chars.getCount(), chars.getBuffer());
    return asArray;
}

struct StringLiteralReplacementPass : InstPassBase
{
    StringLiteralReplacementPass(IRModule* irModule)
        : InstPassBase(irModule)
    {
    }

    bool changed = false;

    void processInst(IRInst* inst)
    {
        if (auto strLitType = as<IRStringLiteralType>(inst))
        {
            IRBuilder builder(module);
            IRBuilderSourceLocRAII srcLocRAII(&builder, inst->sourceLoc);
            builder.setInsertBefore(inst);
            auto arrayType = getStringLiteralArrayType(builder, strLitType);
            inst->replaceUsesWith(arrayType);
            inst->removeAndDeallocate();
            changed = true;
        }
        else if (auto strLit = as<IRStringLit>(inst))
        {
            if (as<IRStringLiteralType>(strLit->getDataType()) ||
                as<IRArrayType>(strLit->getDataType()))
            {
                IRBuilder builder(module);
                IRBuilderSourceLocRAII srcLocRAII(&builder, inst->sourceLoc);
                builder.setInsertBefore(inst);
                auto asArray = getStringLiteralAsArray(builder, strLit);
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

bool replaceStringLiteralsReturnChanged(
    TargetProgram* target,
    IRModule* module,
    StringLiteralsOptions options)
{
    SLANG_UNUSED(target);
    bool res = false;
    if (options.replaceStringLiteralsWithArray)
    {
        StringLiteralReplacementPass pass(module);
        pass.processModule();
        res |= pass.changed;
    }
    return res;
}
} // namespace Slang
