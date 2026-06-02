// slang-ast-modifier.cpp
#include "slang-ast-modifier.h"

#include "slang-ast-expr.h"

namespace Slang
{

void printDiagnosticArg(StringBuilder& sb, Modifier* modifier)
{
    if (!modifier)
        return;
    if (modifier->keywordName && modifier->keywordName->text.getLength())
        sb << modifier->keywordName->text;
    if (auto hlslSemantic = as<HLSLSemantic>(modifier))
        sb << hlslSemantic->name.getContent();
    return;
}

} // namespace Slang
