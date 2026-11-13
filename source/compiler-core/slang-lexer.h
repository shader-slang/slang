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
/// through `add()` or `setTokens()` so borrowed values cannot bypass pool retention.
template<typename T>
struct LexicalList
{
    const T* begin() const { return m_tokens.getBuffer(); }
    const T* end() const
    {
        SLANG_RELEASE_ASSERT(m_tokens.getCount());
        SLANG_RELEASE_ASSERT(m_tokens.getLast().getType() == TokenType::EndOfFile);
        return m_tokens.getBuffer() + m_tokens.getCount() - 1;
    }

    void add(const T& token)
    {
        // Sequences usually share one pool. We inspect other retained pools
        // only when a macro expansion or imported token changes that pool.
        auto pool = token.getPool();
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
        m_tokens.add(token);
    }

    const List<RefPtr<TokenPool>>& getPools() const { return m_pools; }
    const List<T>& getTokens() const { return m_tokens; }

    /// Replaces the sequence and retains every pool referenced by its values.
    void setTokens(const List<T>& tokens)
    {
        // We retain the replacement's pools before releasing the old list, since
        // callers may pass values borrowing storage from one of those pools.
        LexicalList replacement;
        for (const auto& token : tokens)
            replacement.add(token);
        *this = _Move(replacement);
    }

private:
    List<T> m_tokens;
    List<RefPtr<TokenPool>> m_pools;
};

/// An owning EOF-terminated sequence of tokens with attached trivia.
struct TokenList : LexicalList<Token>
{
};

/// An owning EOF-terminated sequence of atomic raw tokens and trivia.
struct LexemeList : LexicalList<Lexeme>
{
};

struct TokenSpan
{
    TokenSpan();
    TokenSpan(TokenList const& tokenList)
        : m_begin(tokenList.begin()), m_end(tokenList.end())
    {
    }

    const Token* begin() const { return m_begin; }
    const Token* end() const { return m_end; }

    int getCount() { return (int)(m_end - m_begin); }

    const Token* m_begin;
    const Token* m_end;
};

struct TokenReader
{
    Token m_nextToken;
    TokenReader();
    explicit TokenReader(TokenSpan const& tokens)
        : m_cursor(tokens.begin()), m_end(tokens.end())
    {
        _updateLookaheadToken();
    }
    explicit TokenReader(TokenList const& tokens)
        : m_cursor(tokens.begin()), m_end(tokens.end())
    {
        _updateLookaheadToken();
    }
    explicit TokenReader(Token const* begin, Token const* end)
        : m_cursor(begin), m_end(end)
    {
        _updateLookaheadToken();
    }
    struct ParsingCursor
    {
        bool operator==(const ParsingCursor& rhs) const
        {
            return tokenReaderCursor == rhs.tokenReaderCursor;
        }
        bool operator!=(const ParsingCursor& rhs) const { return !(*this == rhs); }

        bool isValid() const { return tokenReaderCursor != nullptr; }

        Token nextToken;
        const Token* tokenReaderCursor = nullptr;
    };
    ParsingCursor getCursor()
    {
        ParsingCursor rs;
        rs.nextToken = m_nextToken;
        rs.tokenReaderCursor = m_cursor;
        return rs;
    }
    void setCursor(ParsingCursor cursor)
    {
        m_cursor = cursor.tokenReaderCursor;
        m_nextToken = cursor.nextToken;
    }
    bool isAtCursor(const ParsingCursor& cursor) const
    {
        return cursor.tokenReaderCursor == m_cursor;
    }
    bool isAtEnd() const { return m_cursor == m_end; }
    Token& peekToken();
    TokenType peekTokenType() const;
    SourceLoc peekLoc() const;

    Token advanceToken();

    /// Consumes a matching token and returns its typed value; otherwise returns empty.
    template<typename T>
    T advanceIf()
    {
        auto token = m_nextToken.as<T>();
        if (token)
            advanceToken();
        return token;
    }

    template<typename T>
    bool is() const
    {
        return m_nextToken.is<T>();
    }

    int getCount() { return (int)(m_end - m_cursor); }

    const Token* m_cursor;
    const Token* m_end;
    static Token getEndOfFileToken();

private:
    /// Update the lookahead token in `m_nextToken` to reflect the cursor state
    void _updateLookaheadToken();
};

/// Reads atomic lexemes, including trivia, from an EOF-terminated borrowed sequence.
struct LexemeReader
{
    LexemeReader() = default;
    explicit LexemeReader(const LexemeList& values)
        : LexemeReader(values.begin(), values.end())
    {
    }
    LexemeReader(const Lexeme* begin, const Lexeme* end)
        : m_cursor(begin), m_end(end)
    {
    }
    Lexeme peekToken() const { return m_cursor ? *m_cursor : Lexeme(); }
    TokenType peekTokenType() const { return peekToken().getType(); }
    SourceLoc peekLoc() const { return peekToken().getLoc(); }
    Lexeme advanceToken()
    {
        auto result = peekToken();
        if (m_cursor != m_end)
            ++m_cursor;
        return result;
    }
    bool isAtEnd() const { return m_cursor == m_end; }
    int getCount() const { return m_cursor ? int(m_end - m_cursor) : 0; }
    const Lexeme* m_cursor = nullptr;
    const Lexeme* m_end = nullptr;
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
    Lexeme lexLexeme();

    /// Reads a token with leading and trailing trivia, returning a real EOF at end.
    ///
    /// Trailing trivia ends after the first `NewLine` lexeme. The lexer classifies
    /// trivia from characters without lexing the following raw token, so callers
    /// may change header-name mode or diagnostic suppression before that token.
    Token lexToken();

    /// Enables contextual recognition of a physical <header-name> operand.
    void setHeaderNameMode(bool isEnabled) { m_isHeaderNameMode = isEnabled; }

    /// Reads all tokens with attached trivia, including the terminating EOF.
    TokenList lexAllSemanticTokens();

    /// Reads raw tokens and comments, omitting whitespace and newline trivia.
    LexemeList lexAllMarkupTokens();

    /// Reads all atomic lexemes, including trivia and the terminating EOF.
    LexemeList lexAllTokens();
    LexemeList lexAllLexemes() { return lexAllTokens(); }

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
    bool m_isHeaderNameMode = false;
};


// Helper routines for extracting values from tokens
String getStringLiteralTokenValue(Lexeme const& token, DiagnosticSink* sink);
inline String getStringLiteralTokenValue(Token const& token, DiagnosticSink* sink)
{
    return getStringLiteralTokenValue(token.getRawToken(), sink);
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

// Returns a positive 32-bit code point in range [0, 0xffffffff] on success or
// -1 on failure. A failure is also diagnosed.
//
// See docs/language-reference/expressions-literal.md for the literal format.
IntegerLiteralValue getCharLiteralValue(Token const& token, DiagnosticSink* sink);

} // namespace Slang

#endif
