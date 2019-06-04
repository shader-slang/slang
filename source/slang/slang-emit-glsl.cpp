// slang-emit-glsl.cpp
#include "slang-emit-glsl.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {


void GLSLSourceEmitter::emitIRParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    emitGLSLParameterGroup(varDecl, type);
}

void GLSLSourceEmitter::emitIREntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout)
{
    emitIREntryPointAttributes_GLSL(irFunc, entryPointLayout);
}

} // namespace Slang
