#ifndef SLANG_LEXER_H
#define SLANG_LEXER_H

#include "core/slang-basic.h"
#include "slang-diagnostic-sink.h"

namespace Slang
{
struct NamePool;

// This header defines atomic scanning, token reads with trivia, and stream readers.
// The lexer borrows its selected pool; complete lexical lists retain that pool.
// Every complete list includes EOF. Spans and readers borrow those lists,
// or caller-supplied ranges with an explicit EOF.

/// An EOF-terminated sequence allocated in one explicitly selected token pool.
///
/// The sequence retains that pool. `end()` points at the stored EOF, which ordinary
/// iteration excludes. A default value is uninitialized storage; constructing a
/// sequence with `add()` or `setValues()` requires selecting its pool first.
template<typename T>
struct LexicalList
{
    LexicalList() = default;
    explicit LexicalList(TokenPool* pool)
        : m_pool(pool)
    {
        SLANG_RELEASE_ASSERT(pool);
    }
    const T* begin() const { return m_values.getBuffer(); }
    const T* end() const
    {
        SLANG_RELEASE_ASSERT(m_values.getCount());
        SLANG_RELEASE_ASSERT(m_values.getLast().getTypeTag() == TokenType::EndOfFile);
        return m_values.getBuffer() + m_values.getCount() - 1;
    }
    void add(T value)
    {
        _validateValue(value);
        m_values.add(value);
    }
    TokenPool* getTokenPool() const { return m_pool; }
    const List<T>& getValues() const { return m_values; }

    /// Replaces the values with occurrences from this sequence's pool.
    void setValues(const List<T>& values)
    {
        SLANG_RELEASE_ASSERT(m_pool);
        for (const auto& value : values)
            _validateValue(value);

        // We copy before replacing because callers may pass this sequence's `getValues()`.
        List<T> replacement(values);
        m_values = _Move(replacement);
    }

private:
    void _validateValue(const T& value) const
    {
        SLANG_RELEASE_ASSERT(m_pool);
        SLANG_RELEASE_ASSERT(value.getPool() == m_pool.get());
    }
    RefPtr<TokenPool> m_pool;
    List<T> m_values;
};

/// An owning EOF-terminated sequence of tokens with attached trivia.
struct TokenList : LexicalList<Token>
{
    using LexicalList<Token>::LexicalList;
    const List<Token>& getTokens() const { return getValues(); }
    void setTokens(const List<Token>& tokens) { setValues(tokens); }
};

/// An owning EOF-terminated sequence of atomic raw tokens and trivia.
struct LexemeList : LexicalList<Lexeme>
{
    using LexicalList<Lexeme>::LexicalList;
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
    TokenType peekTokenType() const { return peekToken().getTypeTag(); }
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
    LexemeTypeTag peekLexemeTypeTag() const { return peekLexeme().getTypeTag(); }
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

/// Recognition rules used by a lexer until another mode is selected.
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

/// A stateful scanner of atomic lexemes or tokens with trivia in a borrowed pool.
struct Lexer
{
    /// Initializes scanning over a source view using the supplied lexical pool.
    ///
    /// All returned values belong to `tokenPool`. The caller keeps that pool alive
    /// while the lexer or any borrowed result is used.
    void initialize(
        SourceView* sourceView,
        DiagnosticSink* sink,
        NamePool* namePool,
        TokenPool* tokenPool);

    ~Lexer();

    TokenPool* getTokenPool() const { return m_tokenPool; }

    /// Runs the lexer to try and extract a single token, which is returned.
    /// This can be used by the DiagnosticSink to be able to display more appropriate
    /// information when displaying a source location - such as underscoring the
    /// token at that location.
    ///
    /// NOTE! This function is relatively slow, and is designed for use around this specific
    /// purpose. It does not return a token or a token type, because that information is
    /// not needed by the DiagnosticSink.
    static UnownedStringSlice sourceLocationLexer(const UnownedStringSlice& in);

    /// Reads one atomic lexeme, including trivia, preserving both its physical and logical
    /// spellings.
    Lexeme readLexeme();

    /// Reads a token with leading and trailing trivia, returning a real EOF at end.
    ///
    /// Trailing trivia ends after the first `NewLine` lexeme. The lexer classifies
    /// trivia from characters without lexing the following raw token, so callers
    /// may change header-name mode or diagnostic suppression before that token.
    Token readToken();

    /// Selects recognition rules before the next raw token is read.
    ///
    /// The selected mode remains in effect across reads until another mode is selected.
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

    // Records whether a logical character read removed an escaped newline in the
    // current lexeme. `readLexeme()` resets the flag before recognition and uses it
    // to decide whether to build a logical spelling with those bytes removed.
    bool m_hasLineSpliceInCurrentLexeme = false;
};


// Helper routines for extracting values from tokens
/// Decodes a string literal, including a C++ raw string.
///
/// Requires a non-empty token. Escaped physical newlines have already been removed from its
/// logical spelling. An unterminated ordinary literal yields its decoded prefix; an incomplete
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
