// slang-ir-typeflow-set.h
#pragma once
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

//
// Helpers to iterate over elements of a collection.
//

template<typename F>
void forEachInSet(IRModule* module, IRSetBase* info, F func)
{
    List<IRInst*>& elements = *module->getContainerPool().getList<IRInst>();

    for (UInt i = 0; i < info->getOperandCount(); ++i)
        elements.add(info->getElement(i));

    for (auto element : elements)
        func(element);

    module->getContainerPool().free(&elements);
}

// Upcast the value in 'arg' to match the destInfo type. This method inserts
// any necessary reinterprets or tag translation instructions.
//
IRInst* upcastSet(IRBuilder* builder, IRInst* arg, IRType* destInfo);

} // namespace Slang
