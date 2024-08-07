#pragma once

#include "slang-emit-c-like.h"

namespace Slang
{

class WGSLSourceEmitter : public CLikeSourceEmitter
{
public:

    WGSLSourceEmitter(const Desc& desc)
        : CLikeSourceEmitter(desc)
    {}

    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;
    // TODO: Consider removing this override
    virtual void emitGlobalInstImpl(IRInst* inst) SLANG_OVERRIDE;
    
};

} // namespace Slang
