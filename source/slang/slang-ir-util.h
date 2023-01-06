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


inline IRInst* unwrapAttributedType(IRInst* type)
{
    while (auto attrType = as<IRAttributedType>(type))
        type = attrType->getBaseType();
    return type;
}
}

#endif
