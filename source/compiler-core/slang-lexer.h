#ifndef SLANG_LEXER_H
#define SLANG_LEXER_H

#include "core/slang-basic.h"
#include "slang-diagnostic-sink.h"

namespace Slang
{
struct NamePool;

//

/// An EOF-terminated lexical sequence that retains its referenced pools.
///
/// `end()` points at the stored EOF, so ordinary iteration excludes that sentinel.
/// The sequence must contain EOF before it is exposed to a reader. Mutation goes
/// through `add()` or `setValues()` so borrowed values cannot bypass pool retention.
template<typename T>
struct LexicalList
{
    const T* begin() const { return m_values.getBuffer(); }
    const T* end() const
    {
        SLANG_RELEASE_ASSERT(m_values.getCount());
        SLANG_RELEASE_ASSERT(m_values.getLast().getType() == TokenType::EndOfFile);
        return m_values.getBuffer() + m_values.getCount() - 1;
    }

    void add(const T& value)
    {
        // Sequences usually share one pool. We inspect other retained pools
        // only when a macro expansion or imported token changes that pool.
        auto pool = value.getPool();
        if (pool)
        {
            if (m_pools.getCount() == 0 || m_pools.getLast().get() != pool)
            {
                bool isRetained = false;
                for (const auto& retained : m_pools)
                {
                    if (retained.get() == pool)
                        isRetained = true;
                }
                if (!isRetained)
                    m_pools.add(pool);
            }
        }
        m_values.add(value);
    }

    const List<RefPtr<TokenPool>>& getPools() const { return m_pools; }
    const List<T>& getValues() const { return m_values; }

    /// Replaces the sequence and retains every pool referenced by its values.
    void setValues(const List<T>& values)
    {
        // We retain the replacement's pools before releasing the old list, since
        // callers may pass values borrowing storage from one of those pools.
        LexicalList replacement;
        for (const auto& value : values)
            replacement.add(value);
        *this = _Move(replacement);
    }

private:
    List<T> m_values;
    List<RefPtr<TokenPool>> m_pools;
};

/// An owning EOF-terminated sequence of tokens with attached trivia.
struct TokenList : LexicalList<Token>
{
    const List<Token>& getTokens() const { return getValues(); }
    void setTokens(const List<Token>& tokens) { setValues(tokens); }
};

/// An owning EOF-terminated sequence of atomic raw tokens and trivia.
struct LexemeList : LexicalList<Lexeme>
{
    const List<Lexeme>& getLexemes() const { return getValues(); }
    void setLexemes(const List<Lexeme>& lexemes) { setValues(lexemes); }
};

/// A borrowed range ending immediately before its stored EOF token.
struct TokenSpan
{
    TokenSpan(const TokenList& tokens)
        : m_begin(tokens.begin()), m_end(tokens.end())
    {
    }
    TokenSpan(const Token* begin, const Token* end)
        : m_begin(begin), m_end(end)
    {
        SLANG_RELEASE_ASSERT(begin);
        SLANG_RELEASE_ASSERT(end->is<EndOfFileToken>());
    }

    const Token* begin() const { return m_begin; }
    const Token* end() const { return m_end; }
    int getCount() const { return int(m_end - m_begin); }

private:
    const Token* m_begin = nullptr;
    const Token* m_end = nullptr;
};

/// Reads borrowed tokens and repeats the supplied EOF after reaching the range end.
///
/// Full sequences provide their stored EOF. A bounded subsequence must supply an
/// EOF explicitly; the reader never allocates lexical storage or reinterprets a token.
/// A default reader must be assigned an initialized reader before any token is read.
struct TokenReader
{
    TokenReader() = default;
    explicit TokenReader(const TokenSpan& tokens)
        : TokenReader(tokens.begin(), tokens.end(), tokens.end()->as<EndOfFileToken>())
    {
    }
    explicit TokenReader(const TokenList& tokens)
        : TokenReader(TokenSpan(tokens))
    {
    }
    TokenReader(const Token* begin, const Token* end, EndOfFileToken endOfFile)
        : m_cursor(begin), m_end(end), m_endOfFile(endOfFile)
    {
        SLANG_RELEASE_ASSERT(endOfFile);
        _updateLookaheadToken();
    }

    /// A saved position, including any unconsumed replacement for the current token.
    struct ParsingCursor
    {
        bool operator==(const ParsingCursor& rhs) const
        {
            if (tokenReaderCursor != rhs.tokenReaderCursor)
                return false;
            if (nextToken.getObject() != rhs.nextToken.getObject())
                return false;
            if (nextToken.getLoc() != rhs.nextToken.getLoc())
                return false;
            return nextToken.getFlags() == rhs.nextToken.getFlags();
        }
        bool operator!=(const ParsingCursor& rhs) const { return !(*this == rhs); }
        bool isValid() const { return bool(nextToken); }
        Token nextToken;
        const Token* tokenReaderCursor = nullptr;
    };

    ParsingCursor getCursor() const { return {m_nextToken, m_cursor}; }
    /// Restores a cursor obtained from this reader or a copy of this reader.
    void setCursor(ParsingCursor cursor)
    {
        SLANG_RELEASE_ASSERT(cursor.isValid());
        m_cursor = cursor.tokenReaderCursor;
        m_nextToken = cursor.nextToken;
    }
    bool isAtCursor(const ParsingCursor& cursor) const { return getCursor() == cursor; }
    bool isAtEnd() const { return m_cursor == m_end; }
    Token peekToken() const
    {
        SLANG_RELEASE_ASSERT(m_nextToken);
        return m_nextToken;
    }
    TokenType peekTokenType() const { return peekToken().getType(); }
    SourceLoc peekLoc() const { return peekToken().getLoc(); }
    Token readToken();

    /// Replaces the unconsumed token, for example with the second closer of `>>`.
    ///
    /// Requires a non-EOF token at the current position. Advancing afterward moves
    /// past the original array entry. The caller retains the replacement's pool.
    void replaceLookaheadToken(Token replacement)
    {
        SLANG_RELEASE_ASSERT(!isAtEnd());
        SLANG_RELEASE_ASSERT(replacement);
        SLANG_RELEASE_ASSERT(!replacement.is<EndOfFileToken>());
        m_nextToken = replacement;
    }

    /// Consumes a matching token and returns its typed value; otherwise returns empty.
    template<typename T>
    T advanceIf()
    {
        auto token = peekToken().as<T>();
        if (token)
            readToken();
        return token;
    }
    template<typename T>
    bool is() const
    {
        return peekToken().is<T>();
    }
    int getCount() const { return m_cursor ? int(m_end - m_cursor) : 0; }

private:
    void _updateLookaheadToken() { m_nextToken = m_cursor == m_end ? m_endOfFile : *m_cursor; }
    const Token* m_cursor = nullptr;
    const Token* m_end = nullptr;
    EndOfFileToken m_endOfFile;
    Token m_nextToken;
};

/// Reads atomic lexemes, including trivia, from an EOF-terminated borrowed sequence.
struct LexemeReader
{
    LexemeReader() = default;
    explicit LexemeReader(const LexemeList& values)
        : m_cursor(values.begin()), m_end(values.end())
    {
    }
    Lexeme peekLexeme() const
    {
        SLANG_RELEASE_ASSERT(m_cursor);
        return *m_cursor;
    }
    LexemeKind peekLexemeKind() const { return peekLexeme().getType(); }
    SourceLoc peekLoc() const { return peekLexeme().getLoc(); }
    Lexeme readLexeme()
    {
        auto result = peekLexeme();
        if (m_cursor != m_end)
            ++m_cursor;
        return result;
    }
    bool isAtEnd() const { return m_cursor == m_end; }
    int getCount() const { return m_cursor ? int(m_end - m_cursor) : 0; }

private:
    const Lexeme* m_cursor = nullptr;
    const Lexeme* m_end = nullptr;
};

/// Selects recognition rules for the next raw token requested from a lexer.
enum class LexerMode
{
    Normal,
    ExpectingAngleBracketHeaderName,
};

typedef unsigned int LexerFlags;
enum
{
    kLexerFlag_SuppressDiagnostics = 1
                                     << 2, ///< Suppress errors about invalid/unsupported characters
};

struct Lexer
{
    /// Initializes scanning over a source view using the supplied lexical pool.
    ///
    /// When `tokenPool` is null, the source manager's pool owns the lexical data.
    /// An explicitly supplied pool must remain alive while the lexer is used.
    void initialize(
        SourceView* sourceView,
        DiagnosticSink* sink,
        NamePool* namePool,
        TokenPool* tokenPool = nullptr);

    ~Lexer();

    /// Runs the lexer to try and extract a single token, which is returned.
    /// This can be used by the DiagnosticSink to be able to display more appropriate
    /// information when displaying a source location - such as underscoring the
    /// token at that location.
    ///
    /// NOTE! This function is relatively slow, and is designed for use around this specific
    /// purpose. It does not return a token or a token type, because that information is
    /// not needed by the DiagnosticSink.
    static UnownedStringSlice sourceLocationLexer(const UnownedStringSlice& in);

    /// Reads one atomic lexeme, including trivia, preserving its exact input spelling.
    Lexeme readLexeme();

    /// Reads a token with leading and trailing trivia, returning a real EOF at end.
    ///
    /// Trailing trivia ends after the first `NewLine` lexeme. The lexer classifies
    /// trivia from characters without lexing the following raw token, so callers
    /// may change header-name mode or diagnostic suppression before that token.
    Token readToken();

    /// Selects recognition rules before the next raw token is read.
    void setMode(LexerMode mode) { m_mode = mode; }

    /// Reads all tokens with attached trivia, including the terminating EOF.
    TokenList readAllTokens();

    /// Reads every atomic lexeme, including trivia and the terminating EOF.
    LexemeList readAllLexemes();

    /// Get the diagnostic sink, taking into account flags. Will return null if suppressing
    /// diagnostics.
    DiagnosticSink* getDiagnosticSink()
    {
        return ((m_lexerFlags & kLexerFlag_SuppressDiagnostics) == 0) ? m_sink : nullptr;
    }

    SourceLoc findNextLineEnd(SourceLoc from, UInt& lineCount) const;

    SourceView* m_sourceView;
    DiagnosticSink* m_sink;
    NamePool* m_namePool;

    char const* m_cursor;

    char const* m_begin;
    char const* m_end;

    /// The starting sourceLoc (same as first location of SourceView)
    SourceLoc m_startLoc;

    TokenFlags m_tokenFlags;
    LexerFlags m_lexerFlags;

    TokenPool* m_tokenPool = nullptr;
    LexerMode m_mode = LexerMode::Normal;
};


// Helper routines for extracting values from tokens
/// Decodes a string literal, including a C++ raw string.
///
/// Requires a non-empty token. Escaped physical newlines have already been removed from its
/// canonical text. An unterminated ordinary literal yields its decoded prefix; an incomplete
/// raw-string delimiter yields an empty string. The lexer diagnoses missing delimiters, and
/// this function diagnoses invalid escape sequences when `sink` is non-null.
String getStringLiteralTokenValue(RawStringLiteral token, DiagnosticSink* sink);
inline String getStringLiteralTokenValue(StringLiteralToken token, DiagnosticSink* sink)
{
    return getStringLiteralTokenValue(token.getRawToken().as<RawStringLiteral>(), sink);
}
String getFileNameTokenValue(Token const& token);

// Note about integer literal values. This is the underlying storage type, but
// integer literals are reinterpreted as unsigned when necessary, based on the
// type expression.
typedef int64_t IntegerLiteralValue;
typedef double FloatingPointLiteralValue;

IntegerLiteralValue getIntegerLiteralValue(
    Token const& token,
    DiagnosticSink* sink,
    UnownedStringSlice* outSuffix = 0,
    bool* outIsDecimalBase = 0,
    bool* outHasOverflowed = 0);


// The suffix type of the literal
enum class FloatingPointLiteralType
{
    // Error: bad mantissa
    BadSignificand,

    // Error: bad suffix
    BadSuffix,

    // 16-bit half-float
    Half,

    // 32-bit float
    Float,

    // 64-bit double
    Double,
};

// Reads a floating point value from a literal token.
//
// outLiteralType signals the literal type, including parsing errors. In case of
// a parsing error, outErrorContent contains the bad part (significand or suffix).
//
// When outIsOutOfRange is true, the return value is either:
// - 0        - underflow (between 0 and denormal min)
// - INFINITY - overflow (above maximum value for the literal type)
//
// When outPrecisionLost is true, the significand was truncated. Reported only
// for hex floats and when outIsOutOfRange is false.
//
// The returned value is rounded according to the suffix as reported by
// outLiteralType.
FloatingPointLiteralValue getFloatingPointLiteralValue(
    Token const& token,
    FloatingPointLiteralType& outLiteralType,
    bool& outIsOutOfRange,
    bool& outPrecisionLost,
    UnownedStringSlice& outErrorContent);

/// Decodes a character literal to a 32-bit value, or diagnoses invalid text and returns -1.
///
/// Requires a non-empty token. Escape sequences and UTF-8 characters use the same code-point
/// interpretation; a literal must contain exactly one character between its quotes.
IntegerLiteralValue getCharLiteralValue(RawCharLiteral token, DiagnosticSink* sink);
inline IntegerLiteralValue getCharLiteralValue(CharLiteralToken token, DiagnosticSink* sink)
{
    return getCharLiteralValue(token.getRawToken().as<RawCharLiteral>(), sink);
}

} // namespace Slang

#endif
