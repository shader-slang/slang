#ifndef SLANG_TOKEN_TYPES_H
#define SLANG_TOKEN_TYPES_H

// `slang-token.h` includes this fragment inside `namespace Slang` after declaring `Token`.
// The same descriptors classify atomic tokens and tokens with attached trivia; the
// latter wrappers add only their C++ inheritance and static type information.
// clang-format off
#if 0 // FIDDLE TEMPLATE:
% local lexeme_defs = require("lexeme-defs")
% local function tokenTypeName(class)
%     if class == lexeme_defs.token then return "Token" end
%     return class.name .. "Token"
% end
// This fragment defines typed tokens with attached trivia and no additional stored state.
// `slang-token.h` includes it inside `namespace Slang` after `Token` and the descriptors.
% for _, class in ipairs(lexeme_defs.classes) do
%     if class ~= lexeme_defs.token and class:isSubclassOf(lexeme_defs.token) then
/// A token with attached trivia in the `$(class.name)` class.
struct $(tokenTypeName(class)) : $(tokenTypeName(class.base))
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
#include "slang-token-types.h.fiddle"
#endif // FIDDLE END
// clang-format on

#endif // SLANG_TOKEN_TYPES_H
