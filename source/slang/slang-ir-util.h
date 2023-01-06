// slang-ir-util.h
#ifndef SLANG_IR_UTIL_H_INCLUDED
#define SLANG_IR_UTIL_H_INCLUDED

// This file contains utility functions for operating with Slang IR.
//
#include "slang-ir.h"

namespace Slang
{

bool isPtrToClassType(IRInst* type);

bool isPtrToArrayType(IRInst* type);

// True if ptrType is a pointer type to elementType
bool isPointerOfType(IRInst* ptrType, IRInst* elementType);

// True if ptrType is a pointer type to a type of opCode
bool isPointerOfType(IRInst* ptrType, IROp opCode);

// Builds a dictionary that maps from requirement key to requirement value for `interfaceType`.
Dictionary<IRInst*, IRInst*> buildInterfaceRequirementDict(IRInterfaceType* interfaceType);

bool isComInterfaceType(IRType* type);


IROp getTypeStyle(IROp op);
IROp getTypeStyle(BaseType op);

inline bool isScalarIntegerType(IRType* type)
{
    return getTypeStyle(type->getOp()) == kIROp_IntType;
}

inline bool isChildInstOf(IRInst* inst, IRInst* parent)
{
    while (inst)
    {
        if (inst == parent)
            return true;
        inst = inst->getParent();
    }
    return false;
}

    // Specialize `genericToSpecialize` with the generic parameters defined in `userGeneric`.
    // For example:
    // ```
    // int f<T>(T a);
    // ```
    // will be extended into 
    // ```
    // struct IntermediateFor_f<T> { T t0; }
    // int f_primal<T>(T a, IntermediateFor_f<T> imm);
    // ```
    // Given a user generic `f_primal<T>` and a used value parameterized on the same set of generic parameters
    // `IntermediateFor_f`, `genericToSpecialize` constructs `IntermediateFor_f<T>` (using the parameter list
    // from user generic).
    //
IRInst* specializeWithGeneric(
    IRBuilder& builder, IRInst* genericToSpecialize, IRGeneric* userGeneric);

IRInst* maybeSpecializeWithGeneric(IRBuilder& builder, IRInst* genericToSpecailize, IRInst* userGeneric);

    // For a value inside a generic, create a standalone generic wrapping just the value, and replace the use of
    // the original value with a specialization of the new generic using the current generic arguments if
    // `replaceExistingValue` is true.
    // For example, if we have
    // ```
    //     generic G { param T; v = x(T); f = y(v); return f; }
    // ```
    // hoistValueFromGeneric(G, v) turns the code into:
    // ```
    //     generic G1 { param T1; v1 = x(T); return v1; }
    //     generic G { param T; v = specialize(G1, T); f = y(v); return f; }
    // ```
    // This function returns newly created generic inst.
    // if `value` is not inside any generic, this function makes no change to IR, and returns `value`.
IRInst* hoistValueFromGeneric(
    IRBuilder& builder,
    IRInst* value,
    IRInst*& outSpecializedVal,
    bool replaceExistingValue = false);

// Clear dest and move all chidlren from src to dest.
void moveInstChildren(IRInst* dest, IRInst* src);

inline bool isGenericParam(IRInst* param)
{
    auto parent = param->getParent();
    if (auto block = as<IRBlock>(parent))
        parent = block->getParent();
    if (as<IRGeneric>(parent))
        return true;
    return false;
}

inline IRInst* unwrapAttributedType(IRInst* type)
{
    while (auto attrType = as<IRAttributedType>(type))
        type = attrType->getBaseType();
    return type;
}
}

#endif
