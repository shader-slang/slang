#include "slang-ir-liveness.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

#include "slang-ir-dominators.h"

namespace Slang
{

namespace { // anonymous

struct GLSLLivenessContext
{
        
        /// Process a function in the module
    void processFunction(IRFunc* funcInst);

        /// Process the module
    void processModule();

    GLSLLivenessContext(IRModule* module):
        m_module(module)
    {
        m_sharedBuilder.init(module);
        m_builder.init(m_sharedBuilder);
    }

    void _replaceStart(IRLiveStart* liveStart);
    void _replaceEnd(IRLiveEnd* liveEnd);   
    void _addDecorations(IRLiveBase* live, IRFunc* func);

    IRType* _getType(IRInst* referenced);

    List<IRInst*> m_insts;
    
    IRStringLit* m_startLit = nullptr;
    IRStringLit* m_endLit = nullptr;

    Dictionary<IRType*, IRFunc*> m_startFuncs;
    Dictionary<IRType*, IRFunc*> m_endFuncs;

    IRModule* m_module;
    SharedIRBuilder m_sharedBuilder;
    IRBuilder m_builder;
};

void GLSLLivenessContext::processFunction(IRFunc* funcInst)
{
    // Iterate through blocks in the function, looking for variables to live track
    for (auto block = funcInst->getFirstBlock(); block; block = block->getNextBlock())
    {
        for (auto inst = block->getFirstChild(); inst; inst = inst->getNextInst())
        {
            switch (inst->getOp())
            {
                case kIROp_LiveEnd:
                case kIROp_LiveStart:
                {
                    m_insts.add(inst);
                    break;
                }
                default: break;
            }
        }
    }
}

void GLSLLivenessContext::_addDecorations(IRLiveBase* live, IRFunc* func)
{
    // We need the spirv extension

    // m_builder.addTargetDecoration();

    auto extensionName = UnownedStringSlice::fromLiteral("GL_EXT_spirv_intrinsics");
    m_builder.addRequireGLSLExtensionDecoration(func, extensionName);

    // TODO(JS): We need to add the spirv id number 

    // https://www.khronos.org/registry/SPIR-V/specs/unified1/SPIRV.html#OpLifetimeStart

    int spirvOp = 0;
    switch (live->getOp())
    {
    case kIROp_LiveStart:   
    {
        spirvOp = 256; 
        if (m_startLit)
        {
            m_builder.addNameHintDecoration(func, m_startLit);
        }
        break;
    }
    case kIROp_LiveEnd:     
    {
        if (m_endLit)
        {
            m_builder.addNameHintDecoration(func, m_endLit);
        }
        spirvOp = 257; 
        break;
    }
    default: break;
    }

    SLANG_ASSERT(spirvOp);

    SLANG_UNUSED(spirvOp);
}


IRType* GLSLLivenessContext::_getType(IRInst* referenced)
{
    auto type = referenced->getDataType();

    if (type->getOp() == kIROp_PtrType)
    {
        type = static_cast<IRPtrType*>(type)->getValueType();
    }
    return type;
}

void GLSLLivenessContext::_replaceStart(IRLiveStart* liveStart)
{
    IRType* type = _getType(liveStart->getReferenced());

    IRFunc* func = nullptr;
    
    if (IRFunc** funcPtr = m_startFuncs.TryGetValue(type))
    {
        func = *funcPtr;
    }
    else
    {
        IRType* paramTypes[2] = 
        {
            m_builder.getRefType(type),
            m_builder.getIntType(),
        };

        func = m_builder.createFunc();

        auto funcType = m_builder.getFuncType(SLANG_COUNT_OF(paramTypes), paramTypes, m_builder.getVoidType());
        m_builder.setDataType(func, funcType);

        _addDecorations(liveStart, func);

        m_startFuncs.Add(type, func);
    }

    IRInst* args[] = 
    {
        liveStart->getReferenced(),
        m_builder.getIntValue(m_builder.getIntType(), 0)
    };

    m_builder.setInsertLoc(IRInsertLoc::after(liveStart));
    m_builder.emitCallInst(m_builder.getVoidType(), func, SLANG_COUNT_OF(args), args);

    liveStart->removeAndDeallocate();
}

void GLSLLivenessContext::_replaceEnd(IRLiveEnd* liveEnd)
{
    IRType* type = _getType(liveEnd->getReferenced());

    IRFunc* func = nullptr;

    if (IRFunc** funcPtr = m_endFuncs.TryGetValue(type))
    {
        func = *funcPtr;
    }
    else
    {
        IRType* paramTypes[1] =
        {
            m_builder.getRefType(type)
        };

        func = m_builder.createFunc();

        auto funcType = m_builder.getFuncType(SLANG_COUNT_OF(paramTypes), paramTypes, m_builder.getVoidType());
        m_builder.setDataType(func, funcType);

        _addDecorations(liveEnd, func);

        m_endFuncs.Add(type, func);
    }

    IRInst* args[] =
    {
        liveEnd->getReferenced(),
    };

    m_builder.setInsertLoc(IRInsertLoc::after(liveEnd));
    m_builder.emitCallInst(m_builder.getVoidType(), func, SLANG_COUNT_OF(args), args);

    liveEnd->removeAndDeallocate();
}

void GLSLLivenessContext::processModule()
{
    // When we process liveness, is prior to output for a target
    // So post specialization

    IRModuleInst* moduleInst = m_module->getModuleInst();

    for (IRInst* child : moduleInst->getChildren())
    {
        // We want to find all of the functions, and process them
        if (auto funcInst = as<IRFunc>(child))
        {
            // Then we want to look through their definition
            // inserting instructions that mark the liveness start/end
            processFunction(funcInst);
        }
    }

    if (m_insts.getCount())
    {
        m_startLit = m_builder.getStringValue(UnownedStringSlice::fromLiteral("livenessStart"));
        m_endLit = m_builder.getStringValue(UnownedStringSlice::fromLiteral("livenessEnd"));
    }
    
    for (auto inst : m_insts)
    {
        switch (inst->getOp())
        {
            case kIROp_LiveStart:
            {
                _replaceStart(static_cast<IRLiveStart*>(inst));
                break;
            }
            case kIROp_LiveEnd:
            {
                _replaceEnd(static_cast<IRLiveEnd*>(inst));
                break;
            }
            default: break;
        }
    }
}

} // anonymous

void applyGLSLLiveness(IRModule* module)
{
    GLSLLivenessContext context(module);

    context.processModule();
}

} // namespace Slang
    