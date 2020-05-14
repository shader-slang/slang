// slang-ast-dump.h
#ifndef SLANG_AST_DUMP_H
#define SLANG_AST_DUMP_H

#include "slang-syntax.h"

namespace Slang
{

struct ASTDumpAccess;

struct ASTDumpUtil
{
    template <typename T>
    SLANG_FORCE_INLINE static T& getMember(T& in) { return in; }

    static void dump(NodeBase* node, StringBuilder& out);
    static void dump(Substitutions* subs, StringBuilder& out);
};

} // namespace Slang

#endif
