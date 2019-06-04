// slang-emit-glsl.h
#ifndef SLANG_EMIT_GLSL_H
#define SLANG_EMIT_GLSL_H

#include "slang-emit-c-like.h"

namespace Slang
{

class GLSLSourceEmitter: public CLikeSourceEmitter
{
public:
    typedef CLikeSourceEmitter Super;

    void emitGLSLTextureType(IRTextureType* texType);
    
    GLSLSourceEmitter(const Desc& desc) :
        Super(desc)
    {
    }

protected:

    virtual void emitIRParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitIREntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;
    virtual void emitTextureTypeImpl(IRTextureType* texType) SLANG_OVERRIDE;
    virtual void emitImageTypeImpl(IRGLSLImageType* type) SLANG_OVERRIDE;
};

}
#endif
