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

// Records a temporary argument that must be converted back after a concrete call.
struct ArgumentPackWorkItem
{
    enum Kind
    {
        Pack,
        UpCast,
    } kind = Pack;

    IRInst* dstArg = nullptr;
    IRInst* concreteArg = nullptr;
};

bool isAnyValueType(IRType* type);

// Converts an argument from a type-flow representation to the type required by a concrete callee.
// For an out or inout argument, `packAfterCall` describes the temporary that must be written back.
IRInst* maybeUnpackArg(
    IRBuilder* builder,
    IRType* paramType,
    IRInst* arg,
    ArgumentPackWorkItem& packAfterCall);

// Writes a concrete out or inout temporary back to its type-flow representation.
void writeBackUnpackedArg(IRBuilder* builder, const ArgumentPackWorkItem& item);

// Upcast the value in 'arg' to match the destInfo type. This method inserts
// any necessary reinterprets or tag translation instructions.
//
IRInst* upcastSet(IRBuilder* builder, IRInst* arg, IRType* destInfo);

} // namespace Slang
