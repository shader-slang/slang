#ifndef SLANG_LEXEME_DEFS_H
#define SLANG_LEXEME_DEFS_H

#include "core/slang-basic.h"

namespace Slang
{

enum class LexemeTypeTag : uint8_t;

/// A lexeme class described by its nonempty range of concrete leaf tags.
///
/// The range is half-open: `firstTag` belongs to the class and `getEndTag()` does not.
/// The description is suitable for diagnostics about either a leaf or an abstract class.
struct LexemeType
{
    constexpr LexemeType(LexemeTypeTag firstTag, LexemeTypeTag endTag, const char* description)
        : firstTag(firstTag)
        , tagCount(uint8_t(uint8_t(endTag) - uint8_t(firstTag)))
        , description(description)
    {
        SLANG_RELEASE_ASSERT(uint8_t(firstTag) < uint8_t(endTag));
    }

    /// Whether `tag` belongs to this class.
    constexpr bool contains(LexemeTypeTag tag) const
    {
        // Unsigned subtraction makes tags below `firstTag` wrap above the range width.
        return uint8_t(uint8_t(tag) - uint8_t(firstTag)) < tagCount;
    }

    constexpr LexemeTypeTag getEndTag() const
    {
        return LexemeTypeTag(uint8_t(firstTag) + tagCount);
    }

    LexemeTypeTag firstTag;
    uint8_t tagCount;
    const char* description;
};

// We assign tags only to leaves. A depth-first traversal groups each class's leaves
// together, so class membership requires one subtraction and one comparison.
// Checked-in output lets `compiler-core` build before the Fiddle generator itself.
// clang-format off
#if 0 // FIDDLE TEMPLATE:
% local lexeme_defs = require("lexeme-defs")
% assert(#lexeme_defs.concreteClasses < 256, "LexemeTypeTag needs room for Count in uint8_t")
% local function tagName(tag)
%     if tag == #lexeme_defs.concreteClasses then return "Count" end
%     return lexeme_defs.concreteClasses[tag + 1].name
% end
/// The tags of concrete lexeme types, in depth-first hierarchy order.
enum class LexemeTypeTag : uint8_t
{
% for _, class in ipairs(lexeme_defs.concreteClasses) do
    $(class.name),
% end
    /// One past all valid tags; an empty lexical value returns this sentinel.
    Count,
};

/// Compatibility name for the tag of a token.
using TokenType = LexemeTypeTag;

namespace LexemeTypes
{
% for _, class in ipairs(lexeme_defs.classes) do
inline constexpr LexemeType $(class.name){
    LexemeTypeTag::$(tagName(class.firstTag)),
    LexemeTypeTag::$(tagName(class.endTag)),
    $(string.format("%q", class.description))};
% end
} // namespace LexemeTypes

/// The concrete descriptors in tag order, shared by tag-to-description lookup.
inline constexpr LexemeType kConcreteLexemeTypes[] = {
% for _, class in ipairs(lexeme_defs.concreteClasses) do
    LexemeTypes::$(class.name),
% end
};

/// Whether `tag` identifies a concrete lexeme type.
constexpr bool isValidLexemeTypeTag(LexemeTypeTag tag)
{
    return uint8_t(tag) < uint8_t(LexemeTypeTag::Count);
}

/// Returns the descriptor for a concrete tag.
///
/// Requires a valid tag; `Count` is not a lexeme type.
inline const LexemeType& getLexemeType(LexemeTypeTag tag)
{
    SLANG_RELEASE_ASSERT(isValidLexemeTypeTag(tag));
    return kConcreteLexemeTypes[uint8_t(tag)];
}

/// Returns a concrete tag's diagnostic description, or `<unknown>` for an empty value.
inline const char* TokenTypeToString(TokenType tag)
{
    if (tag == LexemeTypeTag::Count)
        return "<unknown>";
    return getLexemeType(tag).description;
}
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-lexeme-defs.h.fiddle"
#endif // FIDDLE END
// clang-format on

} // namespace Slang

#endif // SLANG_LEXEME_DEFS_H
