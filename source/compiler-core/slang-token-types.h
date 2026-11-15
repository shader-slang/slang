#ifndef SLANG_TOKEN_TYPES_H
#define SLANG_TOKEN_TYPES_H

// `slang-token.h` includes this fragment inside `namespace Slang`, after declaring `Token`.
// We generate the types for tokens with attached trivia from the token branch of the
// conceptual hierarchy. Their kind tags are also used by the corresponding raw token types.

// clang-format off
#if 0 // FIDDLE TEMPLATE:
% for _, class in ipairs(require("source/compiler-core/slang-lexeme-defs.lua").tokenClasses) do
/// A token with attached trivia in the `$(class.name)` branch of the lexeme hierarchy.
struct $(class.tokenTypeName) : $(class.base.tokenTypeName)
{
    static constexpr LexemeKind kind = LexemeKind::$(class.name);

    static bool isKind(LexemeKind candidateKind)
    {
        return isLexemeKind(candidateKind, kind);
    }
};

% end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-token-types.h.fiddle"
#endif // FIDDLE END
// clang-format on

#endif // SLANG_TOKEN_TYPES_H
