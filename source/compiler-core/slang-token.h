// slang-token.h
#ifndef SLANG_TOKEN_H_INCLUDED
#define SLANG_TOKEN_H_INCLUDED

#include "core/slang-array-view.h"
#include "core/slang-basic.h"
#include "slang-lexeme-defs.h"
#include "slang-name.h"
#include "slang-source-loc.h"

#include <type_traits>

namespace Slang
{

// Lexical values have two layers. Small handles carry a location and occurrence
// flags; immutable objects carry both spellings and are shared within a `TokenPool`.
// The generated definitions describe both the atomic lexeme hierarchy and the
// matching hierarchy of tokens with attached trivia.

using TokenFlags = uint8_t;

/// Occurrence context used by preprocessing and parsing.
struct TokenFlag
{
    enum Enum : TokenFlags
    {
        AtStartOfLine = 1 << 0,
        AfterWhitespace = 1 << 1,
    };
};

class TokenPool;
struct Lexeme;
struct RawToken;
struct Trivia;
struct Token;
class TriviaList;

/// Immutable physical and logical spellings and kind shared by lexemes at different locations.
///
/// Objects are created only by their pool. Physical spelling preserves the exact
/// bytes consumed from a `SourceView`, including escaped newlines. Logical spelling
/// removes those escaped newlines; neither form decodes string or character escapes.
/// Synthesized objects use the physical and logical spellings supplied by their factory.
class LexemeObj
{
public:
    LexemeTypeTag getTypeTag() const { return m_typeTag; }
    /// The exact byte sequence representing the lexeme, including escaped newlines.
    UnownedStringSlice getPhysicalSpelling() const { return m_physicalSpelling; }
    /// The lexeme's spelling after escaped newlines have been removed.
    UnownedStringSlice getLogicalSpelling() const { return m_logicalSpelling; }
    Name* getName() const { return m_name; }
    TokenPool* getPool() const { return m_pool; }

private:
    friend class TokenPool;
    LexemeObj(const LexemeObj&) = delete;
    LexemeObj& operator=(const LexemeObj&) = delete;

    LexemeObj(
        TokenPool* pool,
        LexemeTypeTag type,
        UnownedStringSlice physicalSpelling,
        UnownedStringSlice logicalSpelling,
        Name* name)
        : m_pool(pool)
        , m_typeTag(type)
        , m_physicalSpelling(physicalSpelling)
        , m_logicalSpelling(logicalSpelling)
        , m_name(name)
    {
    }

    TokenPool* m_pool;
    LexemeTypeTag m_typeTag;
    UnownedStringSlice m_physicalSpelling;
    UnownedStringSlice m_logicalSpelling;
    Name* m_name;
};

/// One atomic token or trivia occurrence.
///
/// A null object denotes an empty value, independently of its location or flags.
/// The pool must outlive the handle. The pool owns both spellings, so retaining a
/// handle does not require retaining the input text buffer.
struct Lexeme
{
    static constexpr LexemeType type = LexemeTypes::Lexeme;
    Lexeme() = default;
    Lexeme(const LexemeObj* object, SourceLoc loc, TokenFlags flags = 0)
        : m_object(object), m_loc(loc), m_flags(flags)
    {
    }

    explicit operator bool() const { return m_object != nullptr; }
    const LexemeObj* getObject() const { return m_object; }
    TokenPool* getPool() const { return m_object ? m_object->getPool() : nullptr; }
    LexemeTypeTag getTypeTag() const
    {
        return m_object ? m_object->getTypeTag() : LexemeTypeTag::Count;
    }
    SourceLoc getLoc() const { return m_loc; }
    TokenFlags getFlags() const { return m_flags; }
    /// The physical spelling of this occurrence, or an empty slice for an empty value.
    UnownedStringSlice getPhysicalSpelling() const
    {
        return m_object ? m_object->getPhysicalSpelling() : UnownedStringSlice();
    }
    /// The logical spelling of this occurrence, or an empty slice for an empty value.
    UnownedStringSlice getLogicalSpelling() const
    {
        return m_object ? m_object->getLogicalSpelling() : UnownedStringSlice();
    }
    Index getPhysicalSpellingLength() const { return getPhysicalSpelling().getLength(); }
    Index getLogicalSpellingLength() const { return getLogicalSpelling().getLength(); }
    /// Whether the logical spelling contains any bytes; EOF has no spelling bytes.
    bool hasLogicalSpelling() const { return getLogicalSpellingLength() != 0; }
    Name* getName() const { return m_object ? m_object->getName() : nullptr; }
    Name* getNameOrNull() const { return getName(); }

    /// Whether this non-empty value's tag belongs to the requested type's range.
    bool isInstanceOf(LexemeType targetType) const
    {
        return m_object && targetType.contains(getTypeTag());
    }

    template<typename T>
    bool isInstanceOf() const
    {
        static_assert(std::is_base_of_v<Lexeme, T>, "The requested type must derive from Lexeme.");
        return isInstanceOf(T::type);
    }

    template<typename T>
    bool is() const
    {
        return isInstanceOf<T>();
    }

    /// Returns the checked value, or an empty value when the type does not match.
    template<typename T>
    T as() const
    {
        if (!isInstanceOf<T>())
            return T();
        T result;
        static_cast<Lexeme&>(result) = *this;
        return result;
    }

protected:
    const LexemeObj* m_object = nullptr;
    SourceLoc m_loc;
    TokenFlags m_flags = 0;
};

#include "slang-raw-lexeme-types.h"

/// An immutable raw token and its ordered leading and trailing trivia.
///
/// Location and occurrence flags are excluded from interning identity. Every
/// referenced lexeme object belongs to the same pool as this token object.
class TokenObj
{
public:
    const LexemeObj* getRawToken() const { return m_rawToken; }
    ConstArrayView<const LexemeObj*> getLeadingTrivia() const { return m_leadingTrivia; }
    ConstArrayView<const LexemeObj*> getTrailingTrivia() const { return m_trailingTrivia; }
    /// The total physical spelling length, in bytes, of the leading trivia.
    Index getLeadingTriviaPhysicalLength() const { return m_leadingTriviaPhysicalLength; }
    TokenPool* getPool() const { return m_rawToken->getPool(); }

private:
    friend class TokenPool;
    TokenObj(const TokenObj&) = delete;
    TokenObj& operator=(const TokenObj&) = delete;

    TokenObj(
        const LexemeObj* rawToken,
        ConstArrayView<const LexemeObj*> leadingTrivia,
        ConstArrayView<const LexemeObj*> trailingTrivia,
        Index leadingTriviaPhysicalLength)
        : m_rawToken(rawToken)
        , m_leadingTrivia(leadingTrivia)
        , m_trailingTrivia(trailingTrivia)
        , m_leadingTriviaPhysicalLength(leadingTriviaPhysicalLength)
    {
    }

    const LexemeObj* m_rawToken;
    ConstArrayView<const LexemeObj*> m_leadingTrivia;
    ConstArrayView<const LexemeObj*> m_trailingTrivia;
    Index m_leadingTriviaPhysicalLength;
};

/// A token occurrence with attached trivia.
///
/// The location identifies the beginning of the raw token, excluding leading
/// trivia. For a physically lexed cluster, trivia locations follow from physical
/// spelling lengths. Preprocessing must keep a cluster contiguous in its source view or
/// drop transferred trivia when replacing its location.
/// A null object denotes an empty value; EOF always has a non-null object.
struct Token
{
    static constexpr LexemeType type = LexemeTypes::Token;
    Token() = default;
    Token(const TokenObj* object, SourceLoc loc, TokenFlags flags = 0)
        : m_object(object), m_loc(loc), m_flags(flags)
    {
    }

    explicit operator bool() const { return m_object != nullptr; }
    const TokenObj* getObject() const { return m_object; }
    TokenPool* getPool() const { return m_object ? m_object->getPool() : nullptr; }
    TokenType getTypeTag() const
    {
        return m_object ? m_object->getRawToken()->getTypeTag() : TokenType::Count;
    }
    SourceLoc getLoc() const { return m_loc; }
    TokenFlags getFlags() const { return m_flags; }
    /// The raw token's physical spelling, excluding all attached trivia.
    UnownedStringSlice getPhysicalSpelling() const { return getRawToken().getPhysicalSpelling(); }
    /// The raw token's logical spelling, excluding all attached trivia.
    UnownedStringSlice getLogicalSpelling() const { return getRawToken().getLogicalSpelling(); }
    /// Compatibility alias for the raw token's logical spelling.
    UnownedStringSlice getContent() const { return getLogicalSpelling(); }
    Index getPhysicalSpellingLength() const { return getPhysicalSpelling().getLength(); }
    Index getLogicalSpellingLength() const { return getLogicalSpelling().getLength(); }
    /// Whether the logical spelling contains any bytes; EOF has no spelling bytes.
    bool hasLogicalSpelling() const { return getLogicalSpellingLength() != 0; }
    Name* getName() const { return getRawToken().getName(); }
    Name* getNameOrNull() const { return getName(); }
    RawToken getRawToken() const;
    TriviaList getLeadingTrivia() const;
    TriviaList getTrailingTrivia() const;

    /// Returns the same shared token with different preprocessing context flags.
    Token withFlags(TokenFlags flags) const { return Token(m_object, m_loc, flags); }

    /// Whether this non-empty value's tag belongs to the requested type's range.
    bool isInstanceOf(LexemeType targetType) const
    {
        return m_object && targetType.contains(getTypeTag());
    }

    template<typename T>
    bool isInstanceOf() const
    {
        static_assert(std::is_base_of_v<Token, T>, "The requested type must derive from Token.");
        return isInstanceOf(T::type);
    }

    template<typename T>
    bool is() const
    {
        return isInstanceOf<T>();
    }

    /// Returns the checked value, or an empty value when the type does not match.
    template<typename T>
    T as() const
    {
        if (!isInstanceOf<T>())
            return T();
        T result;
        static_cast<Token&>(result) = *this;
        return result;
    }

protected:
    const TokenObj* m_object = nullptr;
    SourceLoc m_loc;
    TokenFlags m_flags = 0;
};

#include "slang-token-types.h"

/// A borrowed sequence of trivia with locations in one contiguous source view.
///
/// Attached trivia exposes kind, both spellings, and position. Its occurrence flags are
/// zero; preprocessing context is retained on the containing token's raw occurrence.
class TriviaList
{
public:
    /// A cursor over trivia objects and their corresponding source locations.
    struct Iterator
    {
        Trivia operator*() const { return Lexeme(*cursor, loc).as<Trivia>(); }
        Iterator& operator++()
        {
            if (loc.isValid())
                loc = loc + (*cursor)->getPhysicalSpelling().getLength();
            ++cursor;
            return *this;
        }
        bool operator!=(const Iterator& other) const { return cursor != other.cursor; }
        const LexemeObj* const* cursor;
        SourceLoc loc;
    };

    TriviaList() = default;
    TriviaList(ConstArrayView<const LexemeObj*> objects, SourceLoc loc)
        : m_objects(objects), m_loc(loc)
    {
    }

    Iterator begin() const { return {m_objects.begin(), m_loc}; }
    Iterator end() const { return {m_objects.end(), SourceLoc()}; }
    Count getCount() const { return m_objects.getCount(); }
    /// Returns trivia by index; sequential iteration avoids rescanning preceding lengths.
    Trivia operator[](Index index) const;
    ConstArrayView<const LexemeObj*> getObjects() const { return m_objects; }

private:
    ConstArrayView<const LexemeObj*> m_objects;
    SourceLoc m_loc;
};

/// A pool that owns and interns immutable lexeme and token objects, text, and trivia arrays.
///
/// Creation copies spelling text and retains `Name` references. Every raw token
/// and trivia object supplied to this pool must have been created by this pool.
/// Handles remain borrowed; `TokenRef` provides ownership when needed.
/// Owning handles require a pool allocated under `RefPtr` ownership.
/// A pool is not safe for simultaneous mutation by multiple threads.
class TokenPool : public RefObject
{
public:
    TokenPool();
    ~TokenPool();
    TokenPool(const TokenPool&) = delete;
    TokenPool& operator=(const TokenPool&) = delete;

    /// Interns a concrete atomic lexeme and returns its located occurrence.
    ///
    /// Requires a concrete kind, and a name whose text equals `logicalSpelling` when supplied.
    /// EOF and `EndOfDirective` require empty physical and logical spellings.
    /// For lexed objects, `physicalSpelling` preserves the consumed `SourceView` bytes and
    /// `logicalSpelling` removes escaped newlines. Synthetic objects use the supplied forms.
    /// Location and flags are retained only on the returned value.
    Lexeme createLexeme(
        LexemeTypeTag kind,
        UnownedStringSlice physicalSpelling,
        UnownedStringSlice logicalSpelling,
        SourceLoc loc = SourceLoc(),
        TokenFlags flags = 0,
        Name* name = nullptr);

    /// Forms a physical cluster whose trivia and raw token occupy adjacent source bytes.
    ///
    /// Requires a non-empty raw token and concrete trivia objects in both arrays.
    /// All supplied objects must belong to this pool.
    Token createToken(
        RawToken rawToken,
        ConstArrayView<const LexemeObj*> leadingTrivia = {},
        ConstArrayView<const LexemeObj*> trailingTrivia = {});

    /// Creates a token with no trivia and identical physical and logical spellings.
    ///
    /// Requires a concrete token kind. The text overload does not intern a `Name`;
    /// the name overload retains the supplied name and uses its text for both spellings.
    Token createToken(
        TokenType kind,
        UnownedStringSlice spelling,
        SourceLoc loc = SourceLoc(),
        TokenFlags flags = 0);
    Token createToken(
        TokenType kind,
        Name* name,
        SourceLoc loc = SourceLoc(),
        TokenFlags flags = 0);
    EndOfFileToken createEndOfFile(SourceLoc loc = SourceLoc(), TokenFlags flags = 0);

    /// Creates the second `>` of a `>>` lexeme at its original byte location.
    ///
    /// Requires an `OpRsh` lexeme with logical spelling `>>` and physical spelling ending
    /// at its second `>`. The returned lexeme has both spellings `>` and no occurrence flags,
    /// because no logical whitespace separates the two closer characters.
    /// An unknown input location remains unknown.
    RawOpGreater createSecondGreaterToken(RawOpRsh rightShift);

private:
    struct Impl;
    Impl* m_impl;
};

/// A typed token together with ownership of the pool backing both spellings.
///
/// Retaining a pool keeps lexical data alive, but does not retain `SourceManager`
/// records used to interpret its source location.
template<typename T = Token>
class TokenRef
{
public:
    TokenRef() = default;
    explicit TokenRef(T token)
        : m_pool(token.getPool()), m_token(token)
    {
    }

    // Rvalues also copy so both handles retain the pool backing their token.
    // A memberwise move would leave the original token without its owning pool.
    TokenRef(const TokenRef&) = default;
    TokenRef& operator=(const TokenRef&) = default;

    explicit operator bool() const { return bool(m_token); }
    T get() const { return m_token; }
    const T* operator->() const { return &m_token; }
    const T& operator*() const { return m_token; }

private:
    RefPtr<TokenPool> m_pool;
    T m_token;
};

inline RawToken Token::getRawToken() const
{
    if (!m_object)
        return RawToken();
    // `TokenPool` validates this relationship when it creates a `TokenObj`.
    // Copying the known raw-token occurrence needs no repeated kind test.
    RawToken result;
    static_cast<Lexeme&>(result) = Lexeme(m_object->getRawToken(), m_loc, m_flags);
    return result;
}

static_assert(
    sizeof(SourceLoc) == sizeof(uint32_t),
    "Occurrence flags rely on a compact SourceLoc.");
static_assert(sizeof(Lexeme) == sizeof(Token), "Both lexical handles have the same value layout.");

} // namespace Slang

#endif
