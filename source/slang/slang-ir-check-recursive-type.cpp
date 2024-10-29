#include "slang-ir-check-recursive-type.h"

#include "slang-ir-util.h"

namespace Slang
{
bool checkTypeRecursionImpl(
    HashSet<IRInst*>& checkedTypes,
    HashSet<IRInst*>& stack,
    IRInst* type,
    IRInst* field,
    DiagnosticSink* sink)
{
    auto visitElementType = [&](IRInst* elementType, IRInst* field) -> bool
    {
        if (!stack.add(elementType))
        {
            sink->diagnose(field ? field : type, Diagnostics::recursiveType, type);
            return false;
        }
        if (checkedTypes.add(elementType))
            checkTypeRecursionImpl(checkedTypes, stack, elementType, field, sink);
        stack.remove(elementType);
        return true;
    };
    if (auto arrayType = as<IRArrayTypeBase>(type))
    {
        return visitElementType(arrayType->getElementType(), field);
    }
    else if (auto structType = as<IRStructType>(type))
    {
        for (auto sfield : structType->getFields())
            if (!visitElementType(sfield->getFieldType(), sfield))
                return false;
    }
    return true;
}

void checkTypeRecursion(HashSet<IRInst*>& checkedTypes, IRInst* type, DiagnosticSink* sink)
{
    HashSet<IRInst*> stack;
    if (checkedTypes.add(type))
    {
        stack.add(type);
        checkTypeRecursionImpl(checkedTypes, stack, type, nullptr, sink);
    }
}

void checkForRecursiveTypes(IRModule* module, DiagnosticSink* sink)
{
    HashSet<IRInst*> checkedTypes;
    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_StructType:
            {
                checkTypeRecursion(checkedTypes, globalInst, sink);
            }
            break;
        }
    }
}

} // namespace Slang
