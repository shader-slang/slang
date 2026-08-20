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

// Casts a value between concrete types and the structural types produced by type-flow.
// The conversion is recursive so semantic wrappers such as differential-pair info can be nested
// inside arrays, tuples, and optionals without requiring call-boundary special cases.
IRInst* castTypeFlowValue(IRBuilder* builder, IRInst* arg, IRType* destInfo);

} // namespace Slang
