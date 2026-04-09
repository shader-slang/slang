#include "slang-ir-legalize-uniform-buffer-load.h"

namespace Slang
{
void legalizeUniformBufferLoad(IRModule* module)
{
    List<IRLoad*> workList;
    auto collectLoads = [&](IRGlobalValueWithCode* code)
    {
        for (auto block : code->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (auto load = as<IRLoad>(inst))
                {
                    auto uniformBufferType =
                        as<IRConstantBufferType>(load->getPtr()->getDataType());
                    if (!uniformBufferType)
                        continue;
                    workList.add(load);
                }
            }
        }
    };
    for (auto inst : module->getFuncs())
        collectLoads(as<IRGlobalValueWithCode>(inst));
    for (auto inst : module->getGlobalVars())
        collectLoads(as<IRGlobalValueWithCode>(inst));

    IRBuilder builder(module);
    for (auto load : workList)
    {
        auto uniformBufferType = as<IRConstantBufferType>(load->getPtr()->getDataType());
        SLANG_ASSERT(uniformBufferType);
        auto structType = as<IRStructType>(uniformBufferType->getElementType());
        if (!structType)
            continue;
        builder.setInsertBefore(load);
        List<IRInst*> fieldLoads;
        for (auto field : structType->getFields())
        {
            auto fieldAddr = builder.emitFieldAddress(
                builder.getPtrType(field->getFieldType()),
                load->getPtr(),
                field->getKey());
            auto fieldLoad = builder.emitLoad(fieldAddr);
            fieldLoads.add(fieldLoad);
        }
        auto makeStruct = builder.emitMakeStruct(structType, fieldLoads);
        load->replaceUsesWith(makeStruct);
    }
}

} // namespace Slang
