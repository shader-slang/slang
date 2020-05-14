// slang-ast-dump.h
#ifndef SLANG_AST_DUMP_H
#define SLANG_AST_DUMP_H

#include "slang-syntax.h"

namespace Slang
{

struct ASTDumpUtil
{
    static void dump(NodeBase* node, StringBuilder& out);
    static void dump(Substitutions* subs, StringBuilder& out);
};

} // namespace Slang

#endif
