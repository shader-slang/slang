#ifndef SLANG_AST_NATURAL_LAYOUT_H
#define SLANG_AST_NATURAL_LAYOUT_H

#include "slang-ast-base.h"

namespace Slang
{

struct ASTNaturalLayoutContext
{
    struct NaturalSize
    {
        Count size;
        Count alignment;
        Count stride;
    };

    SlangResult calcLayout(Type* type, NaturalSize& outSize);

    explicit ASTNaturalLayoutContext(ASTBuilder* astBuilder, DiagnosticSink* sink = nullptr):
        m_astBuilder(astBuilder),
        m_sink(sink)
    {
    }
    
    SlangResult _getInt(IntVal* intVal, Count& outValue);

protected:
    ASTBuilder* m_astBuilder;
    DiagnosticSink* m_sink;
};

} // namespace Slang

#endif
