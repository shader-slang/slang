#include "slang-ir-liveness.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

#include "slang-ir-dominators.h"

namespace Slang
{

namespace { // anonymous

struct GLSLLivenessContext
{
    enum class Kind
    {
        Invalid,
        Start,
        End,
        CountOf,
    };

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

    void _replace(Kind kind, IRLiveBase* liveStart);
    void _addDecorations(Kind kind, IRFunc* func);

    IRType* _getType(IRInst* referenced);

    Kind getKind(IROp op)
    {
        switch (op)
        {
            case kIROp_LiveStart: return Kind::Start;
            case kIROp_LiveEnd:     return Kind::End;
            default: break;
        }
        return Kind::Invalid;
    }

    struct Info
    {
        Dictionary<IRType*, IRFunc*> m_funcs;
        IRStringLit* m_nameLit = nullptr;
        IRInst* m_opValue = nullptr;
    };

    List<IRInst*> m_insts;
    Info m_infos[Index(Kind::CountOf)];
    IRStringLit* m_extensionLit;

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
            if (getKind(inst->getOp()) != Kind::Invalid)
            {
                m_insts.add(inst);
            }
        }
    }
}

void GLSLLivenessContext::_addDecorations(Kind kind, IRFunc* func)
{
    // We might(?) want to add a decoration saying this is GLSL specific, but at this point
    // we can only be in GLSL dependent IR.
    //
    // m_builder.addTargetDecoration();

    // We need the spirv extension
    m_builder.addDecoration(func, kIROp_RequireGLSLExtensionDecoration, m_extensionLit);

    const auto& info = m_infos[Index(kind)];
    if (info.m_nameLit)
    {
        m_builder.addNameHintDecoration(func, info.m_nameLit);
    }

    m_builder.addDecoration(func, kIROp_SPIRVOpDecoration, info.m_opValue);
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

void GLSLLivenessContext::_replace(Kind kind, IRLiveBase* live)
{
    // TODO(JS): Probably better to use a getReferenced method, but this is the easiest way to go for now.
    IRInst* referenced = live->getOperand(0);

    IRType* type = _getType(referenced);

    IRFunc* func = nullptr;
    
    auto& info = m_infos[Index(kind)];

    if (IRFunc** funcPtr = info.m_funcs.TryGetValue(type))
    {
        func = *funcPtr;
    }
    else
    {
        IRType* paramTypes[] = 
        {
            m_builder.getRefType(type),
            m_builder.getIntType(),
        };

        func = m_builder.createFunc();

        auto funcType = m_builder.getFuncType(SLANG_COUNT_OF(paramTypes), paramTypes, m_builder.getVoidType());
        m_builder.setDataType(func, funcType);

        _addDecorations(kind, func);

        info.m_funcs.Add(type, func);
    }
    SLANG_ASSERT(func);

    IRInst* args[] = 
    {
        referenced,
        m_builder.getIntValue(m_builder.getIntType(), 0)
    };

    m_builder.setInsertLoc(IRInsertLoc::after(live));
    m_builder.emitCallInst(m_builder.getVoidType(), func, SLANG_COUNT_OF(args), args);

    live->removeAndDeallocate();
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
    
    // If we didn't find any liveness instructions then we are done
    if (!m_insts.getCount())
    {
        return;
    }

    // Set up some values that will be needed on instructions
    m_extensionLit = m_builder.getStringValue(UnownedStringSlice::fromLiteral("GL_EXT_spirv_intrinsics"));

    // https://www.khronos.org/registry/SPIR-V/specs/unified1/SPIRV.html#OpLifetimeStart

    {
        auto& info = m_infos[Index(Kind::Start)];
        info.m_nameLit = m_builder.getStringValue(UnownedStringSlice::fromLiteral("livenessStart"));
        info.m_opValue = m_builder.getIntValue(m_builder.getIntType(), 256);
    }
    {
        auto& info = m_infos[Index(Kind::End)];
        info.m_nameLit = m_builder.getStringValue(UnownedStringSlice::fromLiteral("livenessEnd"));
        info.m_opValue = m_builder.getIntValue(m_builder.getIntType(), 257);
    }

    // Iterate across instructions, replacing with a call to a generated function (one that just is a declaration defining the SPIR-V op)
    for (auto inst : m_insts)
    {
        const auto kind = getKind(inst->getOp());
        SLANG_ASSERT(kind != Kind::Invalid);
        _replace(kind, static_cast<IRLiveBase*>(inst));
    }
}

} // anonymous

void applyGLSLLiveness(IRModule* module)
{
    GLSLLivenessContext context(module);

    context.processModule();
}

} // namespace Slang
    