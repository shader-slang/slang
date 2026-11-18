#ifndef SLANG_RAW_LEXEME_TYPES_H
#define SLANG_RAW_LEXEME_TYPES_H

// `slang-token.h` includes this fragment inside `namespace Slang` after declaring `Lexeme`.
// C++ naming belongs to this template: trivia gets a `Trivia` suffix, and atomic tokens
// get a `Raw` prefix. The Lua schema supplies only the hierarchy and tag ranges.
// clang-format off
#if 0 // FIDDLE TEMPLATE:
% local lexeme_defs = require("lexeme-defs")
% local function rawTypeName(class)
%     if class == lexeme_defs.root then return "Lexeme" end
%     if class == lexeme_defs.trivia then return "Trivia" end
%     if class:isSubclassOf(lexeme_defs.trivia) then return class.name .. "Trivia" end
%     return "Raw" .. class.name
% end
// This fragment defines typed atomic lexemes without adding stored state.
// `slang-token.h` includes it inside `namespace Slang` after `Lexeme` and the descriptors.
% for _, class in ipairs(lexeme_defs.classes) do
%     if class ~= lexeme_defs.root then
/// An atomic lexeme in the `$(class.name)` class.
struct $(rawTypeName(class)) : $(rawTypeName(class.base))
{
    static constexpr LexemeType type = LexemeTypes::$(class.name);
%         if not class.isAbstract then
    static constexpr LexemeTypeTag tag = LexemeTypeTag::$(class.name);
%         end
};

%     end
% end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-raw-lexeme-types.h.fiddle"
#endif // FIDDLE END
// clang-format on

#endif // SLANG_RAW_LEXEME_TYPES_H
