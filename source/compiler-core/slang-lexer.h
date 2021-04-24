#ifndef SLANG_LEXER_H
#define SLANG_LEXER_H

#include "../core/slang-basic.h"
#include "slang-diagnostic-sink.h"

namespace Slang
{
    struct NamePool;

    //

    struct TokenList
    {
        const Token* begin() const;
        const Token* end() const;

        SLANG_FORCE_INLINE void add(const Token& token) { m_tokens.add(token); }

        List<Token> m_tokens;
    };

    struct TokenSpan
    {
        TokenSpan();
        TokenSpan(
            TokenList const& tokenList)
            : m_begin(tokenList.begin())
            , m_end  (tokenList.end  ())
        {}

        const Token* begin() const { return m_begin; }
        const Token* end  () const { return m_end  ; }

        int getCount() { return (int)(m_end - m_begin); }

        const Token* m_begin;
        const Token* m_end;
    };

    struct TokenReader
    {
        Token m_nextToken;
        TokenReader();
        explicit TokenReader(TokenSpan const& tokens)
            : m_cursor(tokens.begin())
            , m_end   (tokens.end  ())
            , m_nextToken(tokens.begin() ? *tokens.begin() : getEndOfFileToken())
        {}
        explicit TokenReader(TokenList const& tokens)
            : m_cursor(tokens.begin())
            , m_end   (tokens.end  ())
            , m_nextToken(tokens.begin() ? *tokens.begin() : getEndOfFileToken())
        {}
        struct ParsingCursor
        {
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

        int getCount() { return (int)(m_end - m_cursor); }

        const Token* m_cursor;
        const Token* m_end;
        static Token getEndOfFileToken();
    };

    typedef unsigned int LexerFlags;
    enum
    {
        kLexerFlag_InDirective              = 1 << 0, ///< Turn end-of-line and end-of-file into end-of-directive
        kLexerFlag_ExpectFileName           = 1 << 1, ///< Support `<>` style strings for file paths
        kLexerFlag_IgnoreInvalid            = 1 << 2, ///< Suppress errors about invalid/unsupported characters
        kLexerFlag_ExpectDirectiveMessage   = 1 << 3, ///< Don't lexer ordinary tokens, and instead consume rest of line as a string
    };

    struct Lexer
    {
        typedef uint32_t OptionFlags;
        struct OptionFlag
        {
            enum Enum : OptionFlags
            {
                TokenizeComments         = 1 << 0, ///< If set comments will be output to the token stream
            };
        };

        void initialize(
            SourceView*     sourceView,
            DiagnosticSink* sink,
            NamePool*       namePool,
            MemoryArena*    memoryArena,
            OptionFlags     optionFlags = 0);

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

        Token lexToken(LexerFlags extraFlags = 0);

        TokenList lexAllTokens();

            /// Get the diagnostic sink, taking into account flags. Can return nullptr if ignoring invalid
        DiagnosticSink* getDiagnosticSink(LexerFlags flags) { return ((flags & kLexerFlag_IgnoreInvalid) == 0) ? m_sink : nullptr; }

        SourceView*     m_sourceView;
        DiagnosticSink* m_sink;
        NamePool*       m_namePool;

        char const*     m_cursor;

        char const*     m_begin;
        char const*     m_end;

        /// The starting sourceLoc (same as first location of SourceView)
        SourceLoc       m_startLoc;           

        TokenFlags      m_tokenFlags;
        LexerFlags      m_lexerFlags;
        OptionFlags     m_optionFlags;

        MemoryArena*    m_memoryArena;
    };

    
    // Helper routines for extracting values from tokens
    String getStringLiteralTokenValue(Token const& token);
    String getFileNameTokenValue(Token const& token);

    typedef int64_t IntegerLiteralValue;
    typedef double FloatingPointLiteralValue;

    IntegerLiteralValue getIntegerLiteralValue(Token const& token, UnownedStringSlice* outSuffix = 0);
    FloatingPointLiteralValue getFloatingPointLiteralValue(Token const& token, UnownedStringSlice* outSuffix = 0);
}

#endif
