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

    GLSLSourceEmitter(const Desc& desc) :
        Super(desc)
    {
    }

protected:

};

}
#endif
