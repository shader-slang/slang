#ifndef SLANG_RAW_LEXEME_TYPES_H
#define SLANG_RAW_LEXEME_TYPES_H

// `slang-token.h` includes this fragment inside `namespace Slang`, after declaring `Lexeme`.
// We generate the atomic value types in parent-before-child order from the conceptual
// hierarchy. Each type uses `isKind` to recognize its branch without adding value storage.

// clang-format off
#if 0 // FIDDLE TEMPLATE:
% for _, class in ipairs(require("source/compiler-core/slang-lexeme-defs.lua").rawClasses) do
/// An atomic lexeme in the `$(class.name)` branch of the lexeme hierarchy.
struct $(class.rawTypeName) : $(class.base.rawTypeName)
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
#include "slang-raw-lexeme-types.h.fiddle"
#endif // FIDDLE END
// clang-format on

#endif // SLANG_RAW_LEXEME_TYPES_H
