#ifndef RASTER_RENDERER_LEXER_H
#define RASTER_RENDERER_LEXER_H

#include "../core/basic.h"
#include "diagnostics.h"

namespace Slang
{
    struct NamePool;

    //

    struct TokenList
    {
        Token* begin() const;
        Token* end() const;

        List<Token> mTokens;
    };

    struct TokenSpan
    {
        TokenSpan();
        TokenSpan(
            TokenList const& tokenList)
            : mBegin(tokenList.begin())
            , mEnd  (tokenList.end  ())
        {}

        Token* begin() const { return mBegin; }
        Token* end  () const { return mEnd  ; }

        int GetCount() { return (int)(mEnd - mBegin); }

        Token* mBegin;
        Token* mEnd;
    };

    struct TokenReader
    {
        Token nextToken;
        TokenReader();
        explicit TokenReader(TokenSpan const& tokens)
            : mCursor(tokens.begin())
            , mEnd   (tokens.end  ())
            , nextToken(tokens.begin() ? *tokens.begin() : GetEndOfFileToken())
        {}
        explicit TokenReader(TokenList const& tokens)
            : mCursor(tokens.begin())
            , mEnd   (tokens.end  ())
            , nextToken(tokens.begin() ? *tokens.begin() : GetEndOfFileToken())
        {}
        struct ParsingCursor
        {
            Token nextToken;
            Token* tokenReaderCursor = nullptr;
        };
        ParsingCursor getCursor()
        {
            ParsingCursor rs;
            rs.nextToken = nextToken;
            rs.tokenReaderCursor = mCursor;
            return rs;
        }
        void setCursor(ParsingCursor cursor)
        {
            mCursor = cursor.tokenReaderCursor;
            nextToken = cursor.nextToken;
        }
        bool IsAtEnd() const { return mCursor == mEnd; }
        Token& PeekToken();
        TokenType PeekTokenType() const;
        SourceLoc PeekLoc() const;

        Token AdvanceToken();

        int GetCount() { return (int)(mEnd - mCursor); }

        Token* mCursor;
        Token* mEnd;
        static Token GetEndOfFileToken();
    };

    typedef unsigned int LexerFlags;
    enum
    {
        kLexerFlag_InDirective      = 1 << 0, ///< Turn end-of-line and end-of-file into end-of-directive
        kLexerFlag_ExpectFileName   = 1 << 1, ///< Support `<>` style strings for file paths
        kLexerFlag_IgnoreInvalid    = 1 << 2, ///< Suppress errors about invalid/unsupported characters
        kLexerFlag_ExpectDirectiveMessage = 1 << 3, ///< Don't lexer ordinary tokens, and instead consume rest of line as a string
    };

    struct Lexer
    {
        void initialize(
            SourceView*     sourceView,
            DiagnosticSink* sink,
            NamePool*       namePool,
            MemoryArena*    memoryArena);

        ~Lexer();

        Token lexToken(LexerFlags extraFlags = 0);

        TokenList lexAllTokens();

        SourceView*     sourceView;
        DiagnosticSink* sink;
        NamePool*       namePool;

        char const*     cursor;

        char const*     begin;
        char const*     end;

        /// The starting sourceLoc (same as first location of SourceView)
        SourceLoc       startLoc;           

        TokenFlags      tokenFlags;
        LexerFlags      lexerFlags;

        MemoryArena*    memoryArena;
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
