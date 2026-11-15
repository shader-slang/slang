#ifndef SLANG_LEXEME_DEFS_H
#define SLANG_LEXEME_DEFS_H

#include "core/slang-basic.h"

// Lexemes and tokens with attached trivia share the conceptual hierarchy declared in
// `slang-lexeme-defs.lua`. We generate the kind tags and queries together so the C++ type
// wrappers can use the same ancestry relation as the object factory.
// The checked-in outputs allow `compiler-core` to build before `slang-fiddle`.
namespace Slang
{

/// The concrete lexeme kinds and abstract groups in the conceptual lexeme hierarchy.
enum class LexemeKind : uint8_t
{
    /// The absence of a lexeme, returned by an empty value.
    Unknown,

// clang-format off
#if 0 // FIDDLE TEMPLATE:
% for _, class in ipairs(require("source/compiler-core/slang-lexeme-defs.lua").classes) do
    $(class.name),
% end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-lexeme-defs.h.fiddle"
#endif // FIDDLE END
    // clang-format on

    /// One past the last declared kind.
    Count,
};

/// The kind of a token, in the common lexeme kind space.
using TokenType = LexemeKind;

/// Whether `kind` denotes `base` or one of its subtypes.
///
/// `Unknown` and values outside the kind space do not belong to any lexeme class.
inline bool isLexemeKind(LexemeKind kind, LexemeKind base)
{
    if (kind == LexemeKind::Unknown)
        return false;
    if (kind >= LexemeKind::Count)
        return false;

    // We test each ancestor before moving to its parent. Reaching the root without
    // finding `base` proves that `kind` does not belong to the requested branch.
    for (;;)
    {
        if (kind == base)
            return true;
        if (kind == LexemeKind::Lexeme)
            return false;
        switch (kind)
        {
            // clang-format off
#if 0 // FIDDLE TEMPLATE:
% for _, class in ipairs(require("source/compiler-core/slang-lexeme-defs.lua").classes) do
%     if class.base then
        case LexemeKind::$(class.name):
            kind = LexemeKind::$(class.base.name);
            break;
%     end
% end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 1
#include "slang-lexeme-defs.h.fiddle"
#endif // FIDDLE END
       // clang-format on
        default:
            SLANG_UNREACHABLE("Invalid lexeme kind.");
        }
    }
}

/// Whether `kind` identifies a concrete lexeme that the factory can allocate.
inline bool isConcreteLexemeKind(LexemeKind kind)
{
    switch (kind)
    {
        // clang-format off
#if 0 // FIDDLE TEMPLATE:
% for _, class in ipairs(require("source/compiler-core/slang-lexeme-defs.lua").classes) do
    case LexemeKind::$(class.name):
%     if class.isAbstract then
        return false;
%     else
        return true;
%     end
% end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 2
#include "slang-lexeme-defs.h.fiddle"
#endif // FIDDLE END
       // clang-format on
    default:
        return false;
    }
}

/// The diagnostic description of a lexeme kind, or `<unknown>` for `Unknown`.
///
/// Requires `type` to be a declared kind or `Unknown`.
inline char const* TokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::Unknown:
        return "<unknown>";
        // clang-format off
#if 0 // FIDDLE TEMPLATE:
% for _, class in ipairs(require("source/compiler-core/slang-lexeme-defs.lua").classes) do
    case TokenType::$(class.name):
        return $(string.format("%q", class.description));
% end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 3
#include "slang-lexeme-defs.h.fiddle"
#endif // FIDDLE END
       // clang-format on
    default:
        SLANG_UNREACHABLE("Invalid lexeme kind.");
    }
}

} // namespace Slang

#endif // SLANG_LEXEME_DEFS_H
