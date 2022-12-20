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

}

#endif
