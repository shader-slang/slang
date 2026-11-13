// slang-token.h
#ifndef SLANG_TOKEN_H_INCLUDED
#define SLANG_TOKEN_H_INCLUDED

#include "core/slang-array-view.h"
#include "core/slang-basic.h"
#include "slang-name.h"
#include "slang-source-loc.h"

namespace Slang
{

// Lexical values have two layers. Small handles carry a location and occurrence
// flags; immutable objects carry spelling and are shared within a `TokenPool`.
// The generated definitions describe both the atomic lexeme hierarchy and the
// matching hierarchy of tokens with attached trivia.

/// The unified kind space for atomic lexemes and tokens with trivia.
enum class LexemeKind : uint8_t
{
    Unknown,
#define SLANG_LEXEME_KIND(NAME, BASE, CONCRETE, DESCRIPTION) NAME,
#include "slang-lexeme-defs.h"
#undef SLANG_LEXEME_KIND
    Count,
};

using TokenType = LexemeKind;
char const* TokenTypeToString(TokenType type);

/// Tests whether a concrete or abstract kind belongs to a conceptual base kind.
bool isLexemeKind(LexemeKind kind, LexemeKind base);
bool isConcreteLexemeKind(LexemeKind kind);

using TokenFlags = uint8_t;

/// Occurrence context used by preprocessing and parsing.
struct TokenFlag
{
    enum Enum : TokenFlags
    {
        AtStartOfLine = 1 << 0,
        AfterWhitespace = 1 << 1,
        ScrubbingNeeded = 1 << 2,
    };
};

class TokenPool;
struct Lexeme;
struct RawToken;
struct Trivia;
struct Token;
class TriviaList;

/// Immutable spelling and kind shared by lexemes at different locations.
///
/// Objects are created only by their pool. Content is the logical spelling after
/// escaped newlines have been removed; spelling preserves exact `SourceView` input
/// for lexed objects. Synthesized objects use the spellings supplied by their factory.
/// Neither form decodes escapes in a string or character literal.
class LexemeObj
{
public:
    LexemeKind getType() const { return m_type; }
    UnownedStringSlice getSpelling() const { return m_spelling; }
    UnownedStringSlice getContent() const { return m_content; }
    Name* getName() const { return m_name; }
    TokenPool* getPool() const { return m_pool; }

private:
    friend class TokenPool;
    LexemeObj(const LexemeObj&) = delete;
    LexemeObj& operator=(const LexemeObj&) = delete;

    LexemeObj(
        TokenPool* pool,
        LexemeKind type,
        UnownedStringSlice spelling,
        UnownedStringSlice content,
        Name* name)
        : m_pool(pool), m_type(type), m_spelling(spelling), m_content(content), m_name(name)
    {
    }

    TokenPool* m_pool;
    LexemeKind m_type;
    UnownedStringSlice m_spelling;
    UnownedStringSlice m_content;
    Name* m_name;
};

/// One atomic token or trivia occurrence.
///
/// A null object denotes an empty value, independently of its location or flags.
/// The pool must outlive the handle. The pool owns the spelling, so retaining a
/// handle does not require retaining the input text buffer.
struct Lexeme
{
    static bool isKind(LexemeKind kind) { return isLexemeKind(kind, LexemeKind::Lexeme); }
    Lexeme() = default;
    Lexeme(const LexemeObj* object, SourceLoc loc, TokenFlags flags = 0)
        : m_object(object), m_loc(loc), m_flags(flags)
    {
    }

    explicit operator bool() const { return m_object != nullptr; }
    const LexemeObj* getObject() const { return m_object; }
    TokenPool* getPool() const { return m_object ? m_object->getPool() : nullptr; }
    LexemeKind getType() const { return m_object ? m_object->getType() : LexemeKind::Unknown; }
    SourceLoc getLoc() const { return m_loc; }
    TokenFlags getFlags() const { return m_flags; }
    UnownedStringSlice getSpelling() const
    {
        return m_object ? m_object->getSpelling() : UnownedStringSlice();
    }
    UnownedStringSlice getContent() const
    {
        return m_object ? m_object->getContent() : UnownedStringSlice();
    }
    Index getContentLength() const { return getContent().getLength(); }
    Index getSpellingLength() const { return getSpelling().getLength(); }
    bool hasContent() const { return getContentLength() != 0; }
    Name* getName() const { return m_object ? m_object->getName() : nullptr; }
    Name* getNameOrNull() const { return getName(); }

    template<typename T>
    bool is() const
    {
        return m_object && T::isKind(getType());
    }

    /// Returns a matching typed value, or an empty value without changing this handle.
    template<typename T>
    T as() const
    {
        if (!is<T>())
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

// Each wrapper inherits only the handle fields. We construct checked typed
// values by copying the base handle, rather than downcasting a base C++ object.
#define SLANG_RAW_LEXEME(KIND, NAME, BASE)               \
    struct NAME : BASE                                   \
    {                                                    \
        static bool isKind(LexemeKind kind)              \
        {                                                \
            return isLexemeKind(kind, LexemeKind::KIND); \
        }                                                \
    };
#include "slang-lexeme-defs.h"
#undef SLANG_RAW_LEXEME

/// Immutable token spelling and its ordered leading and trailing trivia.
///
/// Location and occurrence flags are excluded from interning identity. Every
/// referenced lexeme object belongs to the same pool as this token object.
class TokenObj
{
public:
    const LexemeObj* getRawToken() const { return m_rawToken; }
    ConstArrayView<const LexemeObj*> getLeadingTrivia() const { return m_leadingTrivia; }
    ConstArrayView<const LexemeObj*> getTrailingTrivia() const { return m_trailingTrivia; }
    Index getLeadingTriviaLength() const { return m_leadingTriviaLength; }
    TokenPool* getPool() const { return m_rawToken->getPool(); }

private:
    friend class TokenPool;
    TokenObj(const TokenObj&) = delete;
    TokenObj& operator=(const TokenObj&) = delete;

    TokenObj(
        const LexemeObj* rawToken,
        ConstArrayView<const LexemeObj*> leadingTrivia,
        ConstArrayView<const LexemeObj*> trailingTrivia,
        Index leadingTriviaLength)
        : m_rawToken(rawToken)
        , m_leadingTrivia(leadingTrivia)
        , m_trailingTrivia(trailingTrivia)
        , m_leadingTriviaLength(leadingTriviaLength)
    {
    }

    const LexemeObj* m_rawToken;
    ConstArrayView<const LexemeObj*> m_leadingTrivia;
    ConstArrayView<const LexemeObj*> m_trailingTrivia;
    Index m_leadingTriviaLength;
};

/// A token occurrence with attached trivia.
///
/// The location identifies the beginning of the raw token, excluding leading
/// trivia. For a physically lexed cluster, trivia locations follow from spelling
/// lengths. Preprocessing must keep a cluster contiguous in its source view or
/// drop transferred trivia when replacing its location.
/// A null object denotes an empty value; EOF always has a non-null object.
struct Token
{
    static bool isKind(LexemeKind kind) { return isLexemeKind(kind, LexemeKind::Token); }
    Token() = default;
    Token(const TokenObj* object, SourceLoc loc, TokenFlags flags = 0)
        : m_object(object), m_loc(loc), m_flags(flags)
    {
    }

    explicit operator bool() const { return m_object != nullptr; }
    const TokenObj* getObject() const { return m_object; }
    TokenPool* getPool() const { return m_object ? m_object->getPool() : nullptr; }
    TokenType getType() const
    {
        return m_object ? m_object->getRawToken()->getType() : TokenType::Unknown;
    }
    SourceLoc getLoc() const { return m_loc; }
    TokenFlags getFlags() const { return m_flags; }
    UnownedStringSlice getContent() const { return getRawToken().getContent(); }
    UnownedStringSlice getSpelling() const { return getRawToken().getSpelling(); }
    Index getContentLength() const { return getContent().getLength(); }
    Index getSpellingLength() const { return getSpelling().getLength(); }
    bool hasContent() const { return getContentLength() != 0; }
    Name* getName() const { return getRawToken().getName(); }
    Name* getNameOrNull() const { return getName(); }
    RawToken getRawToken() const;
    TriviaList getLeadingTrivia() const;
    TriviaList getTrailingTrivia() const;

    /// Replaces this non-empty token's kind through its pool.
    ///
    /// Requires a concrete token kind or `Unknown`. `Unknown` clears the handle;
    /// EOF and `EndOfDirective` create zero-width tokens without trivia. Other kinds
    /// preserve spellings and trivia and require the caller to justify that interpretation.
    void setType(TokenType type);
    /// Replaces both spellings of a non-empty token, preserving its kind and context.
    ///
    /// Discards attached trivia because changing spelling can invalidate its positions.
    /// Clears any cached name; `getName()` returns null afterward.
    /// Zero-width token kinds require empty replacement text.
    void setContent(UnownedStringSlice content);
    /// Replaces a non-empty token's spelling with a name, preserving kind and context.
    ///
    /// Requires a non-null name and discards attached trivia.
    /// Zero-width token kinds require an empty name.
    void setName(Name* name);
    void setFlags(TokenFlags flags) { m_flags = flags; }
    /// Relocates the raw token and discards trivia whose original positions would be lost.
    void setLoc(SourceLoc loc);

    template<typename T>
    bool is() const
    {
        return m_object && T::isKind(getType());
    }

    template<typename T>
    T as() const
    {
        if (!is<T>())
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

#define SLANG_TOKEN_WRAPPER(KIND, NAME, BASE)            \
    struct NAME : BASE                                   \
    {                                                    \
        static bool isKind(LexemeKind kind)              \
        {                                                \
            return isLexemeKind(kind, LexemeKind::KIND); \
        }                                                \
        void setType(TokenType) = delete;                \
    };
#include "slang-lexeme-defs.h"
#undef SLANG_TOKEN_WRAPPER

/// A borrowed sequence of trivia with locations in one contiguous source view.
///
/// Attached trivia exposes kind, spelling, and position. Its occurrence flags are
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
                loc = loc + (*cursor)->getSpelling().getLength();
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

/// Owns and interns immutable lexeme and token objects, their text, and trivia arrays.
///
/// Interning is scoped to this pool. Creation copies all text and imports any
/// supplied lexeme objects from another pool. `Name` references are retained.
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
    /// Requires a concrete kind, and a name whose text equals `content` when supplied.
    /// EOF and `EndOfDirective` require empty original and logical spellings.
    /// For lexed objects, spelling preserves `SourceView` bytes and content is its
    /// already-scrubbed logical spelling. Synthetic objects use the supplied forms.
    /// Location and flags are retained only on the returned value.
    Lexeme createLexeme(
        LexemeKind kind,
        UnownedStringSlice spelling,
        UnownedStringSlice content,
        SourceLoc loc = SourceLoc(),
        TokenFlags flags = 0,
        Name* name = nullptr);

    /// Forms a physical cluster whose trivia and raw token occupy adjacent source bytes.
    ///
    /// Requires a non-empty raw token and concrete trivia objects in both arrays.
    /// Every supplied object is imported into this pool before interning.
    Token createToken(
        RawToken rawToken,
        ConstArrayView<const LexemeObj*> leadingTrivia = {},
        ConstArrayView<const LexemeObj*> trailingTrivia = {});

    /// Creates a token with no trivia and identical original and logical spelling.
    ///
    /// Requires a concrete token kind. The text overload does not intern a `Name`;
    /// the name overload retains the supplied name and uses its text for both spellings.
    Token createToken(
        TokenType kind,
        UnownedStringSlice content,
        SourceLoc loc = SourceLoc(),
        TokenFlags flags = 0);
    Token createToken(
        TokenType kind,
        Name* name,
        SourceLoc loc = SourceLoc(),
        TokenFlags flags = 0);
    Token createEndOfFile(SourceLoc loc = SourceLoc(), TokenFlags flags = 0);

    /// Creates the second `>` of a `>>` lexeme at its original byte location.
    ///
    /// Requires an `OpRsh` lexeme with logical spelling `>>` and original spelling ending
    /// at its second `>`. The returned lexeme has spelling `>` and no occurrence flags,
    /// because no logical whitespace separates the two closer characters.
    /// An unknown input location remains unknown.
    RawOpGreater createSecondGreaterToken(RawToken rightShift);

    /// Copies a cluster into this pool while preserving its location and context.
    Token importToken(Token token);

private:
    /// Imports a lexeme's immutable state when it belongs to a different pool.
    const LexemeObj* _importLexeme(const LexemeObj* object);
    /// Returns same-pool trivia, using scratch storage only when importing is necessary.
    ConstArrayView<const LexemeObj*> _importTrivia(
        ConstArrayView<const LexemeObj*> objects,
        List<const LexemeObj*>& imported);

    struct Impl;
    Impl* m_impl;
};

/// A typed token together with ownership of the pool backing its spelling.
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
