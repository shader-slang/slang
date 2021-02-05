// slang-intrinsic-expand.h
#ifndef SLANG_INTRINSIC_EXPAND_H
#define SLANG_INTRINSIC_EXPAND_H

#include "slang-emit-c-like.h"

namespace Slang
{

/* Handles all the special case handling of expansions of intrinsics. In particular handles the expansion
of the 'special cases' prefixed with '$' */
struct IntrinsicExpandContext
{
    IntrinsicExpandContext(CLikeSourceEmitter* emitter) :
        m_emitter(emitter),
        m_writer(emitter->getSourceWriter())
    {
    }

    void emit(IRCall* inst, IRUse* args, Int argCount, const UnownedStringSlice& intrinsicText);
    
protected:
    const char* _emitSpecial(const char* cursor);

    SourceWriter* m_writer;
    UnownedStringSlice m_text;
    IRUse* m_args = nullptr;
    Int m_argCount = 0;
    Index m_openParenCount = 0;
    CLikeSourceEmitter* m_emitter;
};

}
#endif
