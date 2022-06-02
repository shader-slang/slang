// slang-ir-com-interface.cpp
#include "slang-ir-com-interface.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

struct ComInterfaceLoweringContext
{
    IRModule* m_module;
    DiagnosticSink* m_diagnosticSink;

    ArtifactStyle m_artifactStyle;

    SharedIRBuilder m_sharedBuilder;

    Dictionary<IRType*, IRType*> m_typeMap;

    IRType* _getReplacedInterfaceType(IRInterfaceType* type)
    {
        if (!type->findDecoration<IRComInterfaceDecoration>())
            return nullptr;

        if (m_artifactStyle == ArtifactStyle::Kernel)
        {
            return _getPointerType(type);
        }

        if (auto resultPtr = m_typeMap.TryGetValue(type))
        {
            return *resultPtr;
        }

        IRBuilder builder(m_sharedBuilder);
        builder.setInsertInto(m_module->getModuleInst());

        IRType* result = builder.getComPtrType(type);
   
        m_typeMap.Add(type, result);
        return result;
    }

    IRType* _getPointerType(IRType* type)
    {
        if (auto resultPtr = m_typeMap.TryGetValue(type))
        {
            return *resultPtr;
        }

        IRBuilder builder(m_sharedBuilder);
        builder.setInsertInto(m_module->getModuleInst());

        auto ptrType = builder.getPtrType(type);

        m_typeMap.Add(type, ptrType);
        return ptrType;
    }

    void replaceTypeUses(IRInst* inst, IRInst* newValue)
    {
        List<IRUse*> uses;
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            uses.add(use);
        }
        for (auto use : uses)
        {
            switch (use->getUser()->getOp())
            {
            case kIROp_WitnessTableIDType:
            case kIROp_WitnessTableType:
            case kIROp_ThisType:
            case kIROp_RTTIPointerType:
            case kIROp_RTTIHandleType:
            case kIROp_ComPtrType:
            case kIROp_PtrType:
            {
                // If it's a pointer type it could be because it is a global.
                // We ignore for now and special case around that
                continue;
            }
            default:
                break;
            }
            use->set(newValue);
        }
    }

    IRType* processInterfaceType(IRInterfaceType* type)
    {
        auto result = _getReplacedInterfaceType(type);
        if (result)
        {
            replaceTypeUses(type, result);
        }
        return result;
    }

    void processThisType(IRThisType* type)
    {
        auto comPtrType = processInterfaceType(as<IRInterfaceType>(type->getConstraintType()));
        if (!comPtrType)
            return;
        replaceTypeUses(type, comPtrType);
    }
    void processGlobalVar(IRGlobalVar* globalVar)
    {
        auto allocatedType = globalVar->getDataType();
        auto varType = allocatedType->getValueType();

        if (auto interfaceType = as<IRInterfaceType>(varType))
        {
            if (auto replacedInterfaceType = _getReplacedInterfaceType(interfaceType))
            {
                auto fullType = _getPointerType(replacedInterfaceType);

                if (auto originalRateQualifiedType = as<IRRateQualifiedType>(globalVar->getFullType()))
                {
                    // Add rate qualification
                    IRBuilder builder(m_sharedBuilder);
                    builder.setInsertInto(m_module->getModuleInst());

                    fullType = builder.getRateQualifiedType(originalRateQualifiedType->getRate(), fullType);
                }

                globalVar->setFullType(fullType);
            }
        }
    }

    void processModule()
    {
        List<IRGlobalVar*> globals;

        for (auto child : m_module->getGlobalInsts())
        {
            switch (child->getOp())
            {
                case kIROp_InterfaceType:
                    processInterfaceType(as<IRInterfaceType>(child));
                    break;
                case kIROp_ThisType:
                    processThisType(as<IRThisType>(child));
                    break;
                case kIROp_GlobalVar:
                    processGlobalVar(static_cast<IRGlobalVar*>(child));
                    break;
                default: break;
            }
        }
    }
};

void lowerComInterfaces(IRModule* module, ArtifactStyle artifactStyle, DiagnosticSink* sink)
{
    ComInterfaceLoweringContext context;
    context.m_module = module;
    context.m_diagnosticSink = sink;
    context.m_artifactStyle = artifactStyle;
    context.m_sharedBuilder.init(module);
    return context.processModule();
}

}
