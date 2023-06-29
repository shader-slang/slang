#include "slang-ir-lower-l-value-cast.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-extract-value-from-type.h"
#include "slang-ir-layout.h"

namespace Slang
{

struct LValueCastLoweringContext
{    
    static bool _isGeneric(IRInst* inst)
    {
        // Check if inst is generic
        for (auto cur = inst->getParent(); cur; cur = cur->getParent())
        {
            if (as<IRGeneric>(cur))
                return true;
        }
        return false;
    }

    void _addToWorkList(IRInst* inst)
    {
        if (!_isGeneric(inst) && !m_workList.contains(inst))
        {
            m_workList.add(inst);
        }
    }

    void _processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_InOutImplicitCast:
        case kIROp_OutImplicitCast:
            _processLValueCast(inst);
            break;
        default:
            break;
        }
    }

    void processModule()
    {
        _addToWorkList(m_module->getModuleInst());

        while (m_workList.getCount() != 0)
        {
            IRInst* inst = m_workList.getLast();
            m_workList.removeLast();

            _processInst(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                _addToWorkList(child);
            }
        }
    }

    bool _canReinterpret(IRType* a, IRType* b)
    {
        auto ptrA = as<IRPtrType>(a);
        auto ptrB = as<IRPtrType>(b);

        // They must both be pointers...
        SLANG_ASSERT(ptrA && ptrB);

        a = ptrA->getValueType();
        b = ptrB->getValueType();

        if (a->m_op == b->m_op)
        {
            if (auto matA = as<IRMatrixType>(a))
            {
                auto matB = static_cast<IRMatrixType*>(b);

                if (getIntVal(matA->getColumnCount()) != getIntVal(matB->getColumnCount()))
                {
                    return false;
                }

                a = matA->getElementType();
                b = matB->getElementType();
            }
            else if (auto vecA = as<IRVectorType>(a))
            {
                auto vecB = static_cast<IRVectorType*>(b);

                if (getIntVal(vecA->getElementCount()) != getIntVal(vecB->getElementCount()))
                {
                    return false;
                }

                a = vecA->getElementType();
                b = vecB->getElementType();
            }
        }

        auto basicA = as<IRBasicType>(a);
        auto basicB = as<IRBasicType>(b);

        if (basicA && basicB)
        {
            auto baseA = basicA->getBaseType();
            auto baseB = basicB->getBaseType();

            const auto& infoA = BaseTypeInfo::getInfo(baseA);
            const auto& infoB = BaseTypeInfo::getInfo(baseB);

            // We allow reinterpret case for int type conversions of the same bit size for now
            if (infoA.sizeInBytes == infoB.sizeInBytes && 
                (infoA.flags & infoB.flags & BaseTypeInfo::Flag::Integer))
            {
                return true;
            }
        }

        return false;
    }

    void _processLValueCast(IRInst* inst)
    {
        auto operand = inst->getOperand(0);
        auto fromType = operand->getDataType();
        auto toType = inst->getDataType();

        switch (m_sourceLanguage)
        {
            case SourceLanguage::HLSL:
            {
                // We only allow reinterpret of int conversions of the same size. 
                // HLSL doens't care about any of these so we can just remove the cast
                if (_canReinterpret(fromType, toType))
                {
                    inst->replaceUsesWith(operand);
                    inst->removeAndDeallocate();
                    return;
                }
                break;
            }
            case SourceLanguage::C:
            case SourceLanguage::CPP:
            case SourceLanguage::CUDA:
            {
                if (_canReinterpret(fromType, toType))
                {
                    return;
                }
                break;
            }
            default: break;
        }

        // Okay we are going to replace the implicit casts with temporaries around call sites/uses.
    }


    LValueCastLoweringContext(TargetRequest* targetRequest, IRModule* module):
        m_targetReq(targetRequest),
        m_module(module)
    {
        m_sourceLanguage = _calcIntermediateSourceLanguage(targetRequest);
    }

    // TODO(JS):
    // This would probably be better served elsewhere, as it bakes in the knowledge of *how* 
    // code generate is performed which could change
    static SourceLanguage _calcIntermediateSourceLanguage(TargetRequest* req)
    {
        // If we are emitting directly just do the default thing
        if (req->shouldEmitSPIRVDirectly())
        {
            return SourceLanguage::Unknown;
        }

        switch (req->getTarget())
        {
            case CodeGenTarget::GLSL:
            case CodeGenTarget::GLSL_Vulkan:
            case CodeGenTarget::GLSL_Vulkan_OneDesc:
            // If we aren't emitting directly we are going to output GLSL to feed to GLSLANG
            case CodeGenTarget::SPIRV:
            case CodeGenTarget::SPIRVAssembly:
            {
                return SourceLanguage::GLSL;
            }
            case CodeGenTarget::HLSL:
            case CodeGenTarget::DXBytecode:
            case CodeGenTarget::DXBytecodeAssembly:
            case CodeGenTarget::DXIL:
            case CodeGenTarget::DXILAssembly:
            {
                return SourceLanguage::HLSL;
            }
            case CodeGenTarget::CSource:
            {
                return SourceLanguage::C;
            }
            case CodeGenTarget::ShaderSharedLibrary:
            case CodeGenTarget::ObjectCode:
            case CodeGenTarget::HostExecutable:
            case CodeGenTarget::HostHostCallable:
            case CodeGenTarget::ShaderHostCallable:
            {
                return SourceLanguage::CPP;
            }
            case CodeGenTarget::CPPSource:
            case CodeGenTarget::HostCPPSource:
            case CodeGenTarget::PyTorchCppBinding:
            {
                return SourceLanguage::CPP;
            }
            case CodeGenTarget::CUDAObjectCode:
            case CodeGenTarget::CUDASource:
            case CodeGenTarget::PTX:
            {
                return SourceLanguage::CUDA;
            }
            default: break;
        }


        return SourceLanguage::Unknown;
    }

    // If the source language is not known, will translate into temps and copies
    SourceLanguage m_sourceLanguage = SourceLanguage::Unknown;
    TargetRequest* m_targetReq;
    IRModule* m_module;
    OrderedHashSet<IRInst*> m_workList;
};

void lowerLValueCast(TargetRequest* targetReq, IRModule* module)
{ 
    LValueCastLoweringContext context(targetReq, module);
    context.processModule();
}

} // namespace Slang
