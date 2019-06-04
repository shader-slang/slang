// slang-emit-glsl.cpp
#include "slang-emit-glsl.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {

void GLSLSourceEmitter::emitGLSLTextureType(IRTextureType* texType)
{
    switch (texType->getAccess())
    {
        case SLANG_RESOURCE_ACCESS_READ_WRITE:
        case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
            emitGLSLTextureOrTextureSamplerType(texType, "image");
            break;

        default:
            emitGLSLTextureOrTextureSamplerType(texType, "texture");
            break;
    }
}

void GLSLSourceEmitter::emitIRParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    emitGLSLParameterGroup(varDecl, type);
}

void GLSLSourceEmitter::emitIREntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout)
{
    emitIREntryPointAttributes_GLSL(irFunc, entryPointLayout);
}

void GLSLSourceEmitter::emitTextureTypeImpl(IRTextureType* texType)
{
    emitGLSLTextureType(texType);
}

void GLSLSourceEmitter::emitImageTypeImpl(IRGLSLImageType* type)
{
    emitGLSLImageType(type);
}

} // namespace Slang
