// slang-ast-dump.h
#ifndef SLANG_AST_DUMP_H
#define SLANG_AST_DUMP_H

#include "slang-syntax.h"

#include "slang-emit-source-writer.h"

namespace Slang
{

struct ASTDumpAccess;

struct ASTDumpUtil
{
    enum class Style
    {
        Hierachical,
        Flat,
    };

    static void dump(NodeBase* node, Style style, SourceWriter* writer);
    static void dump(Substitutions* subs, Style style, SourceWriter* writer);
};

} // namespace Slang

#endif
