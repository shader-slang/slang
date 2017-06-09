#ifndef RASTER_RENDERER_LEXER_H
#define RASTER_RENDERER_LEXER_H

#include "../core/basic.h"
#include "diagnostics.h"

namespace Slang
{
    namespace Compiler
    {
        using namespace CoreLib::Basic;

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
            CodePosition PeekLoc() const;

            Token AdvanceToken();

            int GetCount() { return (int)(mEnd - mCursor); }

            Token* mCursor;
            Token* mEnd;
        };

        typedef unsigned int LexerFlags;
        enum
        {
            kLexerFlag_InDirective = 1 << 0,
            kLexerFlag_ExpectFileName = 2 << 0,
        };

        struct Lexer
        {
            Lexer(
                String const&   path,
                String const&   content,
                DiagnosticSink* sink);

            ~Lexer();

            Token lexToken();

            TokenList lexAllTokens();

            String          path;
            String          content;
            DiagnosticSink* sink;

            char const*     cursor;
            char const*     end;
            CodePosition    loc;
            TokenFlags      tokenFlags;
            LexerFlags      lexerFlags;
        };

        // Helper routines for extracting values from tokens
        String getStringLiteralTokenValue(Token const& token);
        String getFileNameTokenValue(Token const& token);
    }
}

#endif