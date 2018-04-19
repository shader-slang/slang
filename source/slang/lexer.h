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
        TokenReader();
        explicit TokenReader(TokenSpan const& tokens)
            : mCursor(tokens.begin())
            , mEnd   (tokens.end  ())
        {}
        explicit TokenReader(TokenList const& tokens)
            : mCursor(tokens.begin())
            , mEnd   (tokens.end  ())
        {}

        bool IsAtEnd() const { return mCursor == mEnd; }
        Token PeekToken() const;
        TokenType PeekTokenType() const;
        SourceLoc PeekLoc() const;

        Token AdvanceToken();

        int GetCount() { return (int)(mEnd - mCursor); }

        Token* mCursor;
        Token* mEnd;
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
            SourceFile*     sourceFile,
            DiagnosticSink* sink,
            NamePool*       namePool);

        ~Lexer();

        Token lexToken(LexerFlags extraFlags = 0);

        TokenList lexAllTokens();

        // Begin overriding the reported locations of tokens,
        // based on a `#line` directives
        void startOverridingSourceLocations(SourceLoc loc);

        // Stop overriding source locations, and go back
        // to reporting source locations in the original file
        void stopOverridingSourceLocations();

        SourceFile*     sourceFile;
        DiagnosticSink* sink;
        NamePool*       namePool;

        char const*     cursor;

        char const*     begin;
        char const*     end;

        // The starting source location for the code as written,
        // which cannot be overridden.
        SourceLoc       spellingStartLoc;

        // The nominal starting location for the file, taking
        // any active `#line` directive into account.
        SourceLoc       presumedStartLoc;

        TokenFlags      tokenFlags;
        LexerFlags      lexerFlags;
    };

    // Helper routines for extracting values from tokens
    String getStringLiteralTokenValue(Token const& token);
    String getFileNameTokenValue(Token const& token);

    typedef int64_t IntegerLiteralValue;
    typedef double FloatingPointLiteralValue;

    IntegerLiteralValue getIntegerLiteralValue(Token const& token, String* outSuffix = 0);
    FloatingPointLiteralValue getFloatingPointLiteralValue(Token const& token, String* outSuffix = 0);
}

#endif
