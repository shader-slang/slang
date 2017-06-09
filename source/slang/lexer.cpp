#include "Lexer.h"

#include <assert.h>

namespace Slang
{
    namespace Compiler
    {
        static Token GetEndOfFileToken()
        {
            return Token(TokenType::EndOfFile, "", 0, 0, 0, "");
        }

        Token* TokenList::begin() const
        {
            assert(mTokens.Count());
            return &mTokens[0];
        }

        Token* TokenList::end() const
        {
            assert(mTokens.Count());
            assert(mTokens[mTokens.Count()-1].Type == TokenType::EndOfFile);
            return &mTokens[mTokens.Count() - 1];
        }

        TokenSpan::TokenSpan()
            : mBegin(NULL)
            , mEnd  (NULL)
        {}

        TokenReader::TokenReader()
            : mCursor(NULL)
            , mEnd   (NULL)
        {}


        Token TokenReader::PeekToken() const
        {
            if (!mCursor)
                return GetEndOfFileToken();

            Token token = *mCursor;
            if (mCursor == mEnd)
                token.Type = TokenType::EndOfFile;
            return token;
        }

        TokenType TokenReader::PeekTokenType() const
        {
            if (mCursor == mEnd)
                return TokenType::EndOfFile;
            assert(mCursor);
            return mCursor->Type;
        }

        CodePosition TokenReader::PeekLoc() const
        {
            if (!mCursor)
                return CodePosition();
            assert(mCursor);
            return mCursor->Position;
        }

        Token TokenReader::AdvanceToken()
        {
            if (!mCursor)
                return GetEndOfFileToken();

            Token token = *mCursor;
            if (mCursor == mEnd)
                token.Type = TokenType::EndOfFile;
            else
                mCursor++;
            return token;
        }

        // Lexer

        Lexer::Lexer(
            String const&   path,
            String const&   content,
            DiagnosticSink* sink)
            : path(path)
            , content(content)
            , sink(sink)
        {
            cursor  = content.begin();
            end     = content.end();

            loc = CodePosition(1, 1, 0, path);
            tokenFlags = TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace;
            lexerFlags = 0;
        }

        Lexer::~Lexer()
        {
        }

        enum { kEOF = -1 };

        static int peek(Lexer* lexer)
        {
            if(lexer->cursor == lexer->end)
                return kEOF;

            return *lexer->cursor;
        }

        static int advance(Lexer* lexer)
        {
            if(lexer->cursor == lexer->end)
                return kEOF;

            lexer->loc.Col++;
            lexer->loc.Pos++;

            return *lexer->cursor++;
        }

        static void handleNewLine(Lexer* lexer)
        {
            int c = advance(lexer);
            assert(c == '\n' || c == '\r');

            int d = peek(lexer);
            if( (c ^ d) == ('\n' ^ '\r') )
            {
                advance(lexer);
            }

            lexer->loc.Line++;
            lexer->loc.Col = 1;
        }

        static void lexLineComment(Lexer* lexer)
        {
            for(;;)
            {
                switch(peek(lexer))
                {
                case '\n': case '\r': case kEOF:
                    return;

                default:
                    advance(lexer);
                    continue;
                }
            }
        }

        static void lexBlockComment(Lexer* lexer)
        {
            for(;;)
            {
                switch(peek(lexer))
                {
                case kEOF:
                    // TODO(tfoley) diagnostic!
                    return;
                    
                case '\n': case '\r':
                    handleNewLine(lexer);
                    continue;

                case '*':
                    advance(lexer);
                    switch( peek(lexer) )
                    {
                    case '/':
                        advance(lexer);
                        return;

                    default:
                        continue;
                    }

                default:
                    advance(lexer);
                    continue;
                }
            }
        }

        static void lexHorizontalSpace(Lexer* lexer)
        {
            for(;;)
            {
                switch(peek(lexer))
                {
                case ' ': case '\t':
                    advance(lexer);
                    continue;

                default:
                    return;
                }
            }
        }

        static void lexIdentifier(Lexer* lexer)
        {
            for(;;)
            {
                int c = peek(lexer);
                if(('a' <= c ) && (c <= 'z')
                    || ('A' <= c) && (c <= 'Z')
                    || ('0' <= c) && (c <= '9')
                    || (c == '_'))
                {
                    advance(lexer);
                    continue;
                }

                return;
            }
        }

        static void lexDigits(Lexer* lexer, int base)
        {
            for(;;)
            {
                int c = peek(lexer);

                int digitVal = 0;
                switch(c)
                {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    digitVal = c - '0';
                    break;

                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                    if(base <= 10) return;
                    digitVal = 10 + c - 'a';
                    break;

                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                    if(base <= 10) return;
                    digitVal = 10 + c - 'A';
                    break;

                default:
                    // Not more digits!
                    return;
                }

                if(digitVal >= base)
                {
                    char buffer[] = { (char) c, 0 };
                    lexer->sink->diagnose(lexer->loc, Diagnostics::invalidDigitForBase, buffer, base);
                }

                advance(lexer);
            }
        }

        static TokenType maybeLexNumberSuffix(Lexer* lexer, TokenType tokenType)
        {
            // First check for suffixes that
            // indicate a floating-point number
            switch(peek(lexer))
            {
            case 'f': case 'F':
                advance(lexer);
                return TokenType::DoubleLiterial;

            default:
                break;
            }

            // Once we've ruled out floating-point
            // suffixes, we can check for the inter cases

            // TODO: allow integer suffixes in any order...

            // Leading `u` or `U` for unsigned
            switch(peek(lexer))
            {
            default:
                break;

            case 'u': case 'U':
                advance(lexer);
                break;
            }

            // Optional `l`, `L`, `ll`, or `LL`
            switch(peek(lexer))
            {
            default:
                break;

            case 'l': case 'L':
                advance(lexer);
                switch(peek(lexer))
                {
                default:
                    break;

                case 'l': case 'L':
                    advance(lexer);
                    break;
                }
                break;
            }

            return tokenType;
        }

        static bool maybeLexNumberExponent(Lexer* lexer, int base)
        {
            switch( peek(lexer) )
            {
            default:
                return false;

            case 'e': case 'E':
                if(base != 10) return false;
                advance(lexer);
                break;

            case 'p': case 'P':
                if(base != 16) return false;
                advance(lexer);
                break;
            }

            // we saw an exponent marker, so we must 
            switch( peek(lexer) )
            {
            case '+': case '-':
                advance(lexer);
                break;
            }

            // TODO(tfoley): it would be an error to not see digits here...

            lexDigits(lexer, 10);

            return true;
        }

        static TokenType lexNumberAfterDecimalPoint(Lexer* lexer, int base)
        {
            lexDigits(lexer, base);
            maybeLexNumberExponent(lexer, base);
            
            return maybeLexNumberSuffix(lexer, TokenType::DoubleLiterial);
        }

        static TokenType lexNumber(Lexer* lexer, int base)
        {
            // TODO(tfoley): Need to consider whehter to allow any kind of digit separator character.

            TokenType tokenType = TokenType::IntLiterial;

            // At the start of things, we just concern ourselves with digits
            lexDigits(lexer, base);

            if( peek(lexer) == '.' )
            {
                tokenType = TokenType::DoubleLiterial;

                advance(lexer);
                lexDigits(lexer, base);
            }

            if( maybeLexNumberExponent(lexer, base))
            {
                tokenType = TokenType::DoubleLiterial;
            }

            maybeLexNumberSuffix(lexer, tokenType);
            return tokenType;
        }

        static void lexStringLiteralBody(Lexer* lexer, char quote)
        {
            for(;;)
            {
                int c = peek(lexer);
                if(c == quote)
                {
                    advance(lexer);
                    return;
                }

                switch(c)
                {
                case kEOF:
                    lexer->sink->diagnose(lexer->loc, Diagnostics::endOfFileInLiteral);
                    return;

                case '\n': case '\r':
                    lexer->sink->diagnose(lexer->loc, Diagnostics::newlineInLiteral);
                    return;

                case '\\':
                    // Need to handle various escape sequence cases
                    advance(lexer);
                    switch(peek(lexer))
                    {
                    case '\'':
                    case '\"':
                    case '\\':
                    case '?':
                    case 'a':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                        advance(lexer);
                        break;

                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7':
                        // octal escape: up to 3 characters
                        advance(lexer);
                        for(int ii = 0; ii < 3; ++ii)
                        {
                            int d = peek(lexer);
                            if(('0' <= d) && (d <= '7'))
                            {
                                advance(lexer);
                                continue;
                            }
                            else
                            {
                                break;
                            }
                        }
                        break;

                    case 'x':
                        // hexadecimal escape: any number of characters
                        advance(lexer);
                        for(;;)
                        {
                            int d = peek(lexer);
                            if(('0' <= d) && (d <= '9')
                                || ('a' <= d) && (d <= 'f')
                                || ('A' <= d) && (d <= 'F'))
                            {
                                advance(lexer);
                                continue;
                            }
                            else
                            {
                                break;
                            }
                        }
                        break;

                    // TODO: Unicode escape sequences

                    }
                    break;

                default:
                    advance(lexer);
                    continue;
                }
            }
        }

        String getStringLiteralTokenValue(Token const& token)
        {
            assert(token.Type == TokenType::StringLiterial
                || token.Type == TokenType::CharLiterial);

            char const* cursor = token.Content.begin();
            char const* end = token.Content.end();

            auto quote = *cursor++;
            assert(quote == '\'' || quote == '"');

            StringBuilder valueBuilder;
            for(;;)
            {
                assert(cursor != end);

                auto c = *cursor++;

                // If we see a closing quote, then we are at the end of the string literal
                if(c == quote)
                {
                    assert(cursor == end);
                    return valueBuilder.ProduceString();
                }

                // Charcters that don't being escape sequences are easy;
                // just append them to the buffer and move on.
                if(c != '\\')
                {
                    valueBuilder.Append(c);
                    continue;
                }

                // Now we look at another character to figure out the kind of
                // escape sequence we are dealing with:

                int d = *cursor++;

                switch(d)
                {
                // Simple characters that just needed to be escaped
                case '\'':
                case '\"':
                case '\\':
                case '?':
                    valueBuilder.Append(d);
                    continue;

                // Traditional escape sequences for special characters
                case 'a': valueBuilder.Append('\a'); continue;
                case 'b': valueBuilder.Append('\b'); continue;
                case 'f': valueBuilder.Append('\f'); continue;
                case 'n': valueBuilder.Append('\n'); continue;
                case 'r': valueBuilder.Append('\r'); continue;
                case 't': valueBuilder.Append('\t'); continue;
                case 'v': valueBuilder.Append('\v'); continue;

                // Octal escape: up to 3 characterws
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7':
                    {
                        cursor--;
                        int value = 0;
                        for(int ii = 0; ii < 3; ++ii)
                        {
                            d = *cursor;
                            if(('0' <= d) && (d <= '7'))
                            {
                                value = value*8 + (d - '0');

                                cursor++;
                                continue;
                            }
                            else
                            {
                                break;
                            }
                        }

                        // TODO: add support for appending an arbitrary code point?
                        valueBuilder.Append((char) value);
                    }
                    continue;

                // Hexadecimal escape: any number of characters
                case 'x':
                    {
                        cursor--;
                        int value = 0;
                        for(;;)
                        {
                            d = *cursor++;
                            int digitValue = 0;
                            if(('0' <= d) && (d <= '9'))
                            {
                                digitValue = d - '0';
                            }
                            else if( ('a' <= d) && (d <= 'f') )
                            {
                                digitValue = d - 'a';
                            }
                            else if( ('A' <= d) && (d <= 'F') )
                            {
                                digitValue = d - 'A';
                            }
                            else
                            {
                                cursor--;
                                break;
                            }

                            value = value*16 + digitValue;
                        }

                        // TODO: add support for appending an arbitrary code point?
                        valueBuilder.Append((char) value);
                    }
                    continue;

                // TODO: Unicode escape sequences

                }
            }
        }

        String getFileNameTokenValue(Token const& token)
        {
            // A file name usually doesn't process escape sequences
            // (this is import on Windows, where `\\` is a valid
            // path separator cahracter).

            // Just trim off the first and last characters to remove the quotes
            // (whether they were `""` or `<>`.
            return token.Content.SubString(1, token.Content.Length()-2);
        }



        static TokenType lexTokenImpl(Lexer* lexer)
        {
            switch(peek(lexer))
            {
            default:
                break;

            case kEOF:
                if((lexer->lexerFlags & kLexerFlag_InDirective) != 0)
                    return TokenType::EndOfDirective;
                return TokenType::EndOfFile;

            case '\r': case '\n':
                if((lexer->lexerFlags & kLexerFlag_InDirective) != 0)
                    return TokenType::EndOfDirective;
                handleNewLine(lexer);
                return TokenType::NewLine;

            case ' ': case '\t':
                lexHorizontalSpace(lexer);
                return TokenType::WhiteSpace;

            case '.':
                advance(lexer);
                switch(peek(lexer))
                {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    return lexNumberAfterDecimalPoint(lexer, 10);

                // TODO(tfoley): handle ellipsis (`...`)

                default:
                    return TokenType::Dot;
                }

                      case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return lexNumber(lexer, 10);

            case '0':
                {
                    auto loc = lexer->loc;
                    advance(lexer);
                    switch(peek(lexer))
                    {
                    default:
                        return TokenType::IntLiterial;

                    case '.':
                        advance(lexer);
                        return lexNumberAfterDecimalPoint(lexer, 10);

                    case 'x': case 'X':
                        advance(lexer);
                        return lexNumber(lexer, 16);

                    case 'b': case 'B':
                        advance(lexer);
                        return lexNumber(lexer, 2);

                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                        lexer->sink->diagnose(loc, Diagnostics::octalLiteral);
                        return lexNumber(lexer, 8);
                    }
                }

            case 'a': case 'b': case 'c': case 'd': case 'e':
            case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o':
            case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y':
            case 'z':
            case 'A': case 'B': case 'C': case 'D': case 'E':
            case 'F': case 'G': case 'H': case 'I': case 'J':
            case 'K': case 'L': case 'M': case 'N': case 'O':
            case 'P': case 'Q': case 'R': case 'S': case 'T':
            case 'U': case 'V': case 'W': case 'X': case 'Y':
            case 'Z':
            case '_':
                lexIdentifier(lexer);
                return TokenType::Identifier;

            case '\"':
                advance(lexer);
                lexStringLiteralBody(lexer, '\"');
                return TokenType::StringLiterial;

            case '\'':
                advance(lexer);
                lexStringLiteralBody(lexer, '\'');
                return TokenType::CharLiterial;

            case '+':
                advance(lexer);
                switch(peek(lexer))
                {
                case '+': advance(lexer); return TokenType::OpInc;
                case '=': advance(lexer); return TokenType::OpAddAssign;
                default:
                    return TokenType::OpAdd;
                }

            case '-':
                advance(lexer);
                switch(peek(lexer))
                {
                case '-': advance(lexer); return TokenType::OpDec;
                case '=': advance(lexer); return TokenType::OpSubAssign;
                case '>': advance(lexer); return TokenType::RightArrow;
                default:
                    return TokenType::OpSub;
                }

            case '*':
                advance(lexer);
                switch(peek(lexer))
                {
                case '=': advance(lexer); return TokenType::OpMulAssign;
                default:
                    return TokenType::OpMul;
                }

            case '/':
                advance(lexer);
                switch(peek(lexer))
                {
                case '=': advance(lexer); return TokenType::OpDivAssign;
                case '/': advance(lexer); lexLineComment(lexer); return TokenType::LineComment;
                case '*': advance(lexer); lexBlockComment(lexer); return TokenType::BlockComment;
                default:
                    return TokenType::OpDiv;
                }

            case '%':
                advance(lexer);
                switch(peek(lexer))
                {
                case '=': advance(lexer); return TokenType::OpModAssign;
                default:
                    return TokenType::OpMod;
                }

            case '|':
                advance(lexer);
                switch(peek(lexer))
                {
                case '|': advance(lexer); return TokenType::OpOr;
                case '=': advance(lexer); return TokenType::OpOrAssign;
                default:
                    return TokenType::OpBitOr;
                }

            case '&':
                advance(lexer);
                switch(peek(lexer))
                {
                case '&': advance(lexer); return TokenType::OpAnd;
                case '=': advance(lexer); return TokenType::OpAndAssign;
                default:
                    return TokenType::OpBitAnd;
                }

            case '^':
                advance(lexer);
                switch(peek(lexer))
                {
                case '=': advance(lexer); return TokenType::OpXorAssign;
                default:
                    return TokenType::OpBitXor;
                }

            case '>':
                advance(lexer);
                switch(peek(lexer))
                {
                case '>':
                    advance(lexer);
                    switch(peek(lexer))
                    {
                    case '=': advance(lexer); return TokenType::OpShrAssign;
                    default: return TokenType::OpRsh;
                    }
                case '=': advance(lexer); return TokenType::OpGeq;
                default:
                    return TokenType::OpGreater;
                }

            case '<':
                advance(lexer);
                switch(peek(lexer))
                {
                case '<':
                    advance(lexer);
                    switch(peek(lexer))
                    {
                    case '=': advance(lexer); return TokenType::OpShlAssign;
                    default: return TokenType::OpLsh;
                    }
                case '=': advance(lexer); return TokenType::OpLeq;
                default:
                    return TokenType::OpLess;
                }

            case '=':
                advance(lexer);
                switch(peek(lexer))
                {
                case '=': advance(lexer); return TokenType::OpEql;
                default:
                    return TokenType::OpAssign;
                }

            case '!':
                advance(lexer);
                switch(peek(lexer))
                {
                case '=': advance(lexer); return TokenType::OpNeq;
                default:
                    return TokenType::OpNot;
                }

            case '#':
                advance(lexer);
                switch(peek(lexer))
                {
                case '#': advance(lexer); return TokenType::PoundPound;
                default:
                    return TokenType::Pound;
                }

            case '~': advance(lexer); return TokenType::OpBitNot;

            case ':': advance(lexer); return TokenType::Colon;
            case ';': advance(lexer); return TokenType::Semicolon;
            case ',': advance(lexer); return TokenType::Comma;

            case '{': advance(lexer); return TokenType::LBrace;
            case '}': advance(lexer); return TokenType::RBrace;
            case '[': advance(lexer); return TokenType::LBracket;
            case ']': advance(lexer); return TokenType::RBracket;
            case '(': advance(lexer); return TokenType::LParent;
            case ')': advance(lexer); return TokenType::RParent;

            case '?': advance(lexer); return TokenType::QuestionMark;
            case '@': advance(lexer); return TokenType::At;
            case '$': advance(lexer); return TokenType::Dollar;

            }

            // TODO(tfoley): If we ever wanted to support proper Unicode
            // in identifiers, etc., then this would be the right place
            // to perform a more expensive dispatch based on the actual
            // code point (and not just the first byte).

            {
                // If none of the above cases matched, then we have an
                // unexpected/invalid character.

                auto loc = lexer->loc;
                auto sink = lexer->sink;
                int c = advance(lexer);
                if(c >= 0x20 && c <=  0x7E)
                {
                    char buffer[] = { (char) c, 0 };
                    sink->diagnose(loc, Diagnostics::illegalCharacterPrint, buffer);
                }
                else
                {
                    // Fallback: print as hexadecimal
                    sink->diagnose(loc, Diagnostics::illegalCharacterHex, String((unsigned char)c, 16));
                }

                return TokenType::Invalid;
            }
        }

        Token Lexer::lexToken()
        {
            auto flags = this->tokenFlags;
            for(;;)
            {
                Token token;
                token.Position = loc;

                char const* textBegin = cursor;

                auto tokenType = lexTokenImpl(this);

                // The low-level lexer produces tokens for things we want
                // to ignore, such as white space, so we skip them here.
                switch(tokenType)
                {
                case TokenType::Invalid:
                    flags = 0;
                    continue;

                case TokenType::NewLine:
                    flags = TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace;
                    continue;

                case TokenType::WhiteSpace:
                case TokenType::LineComment:
                case TokenType::BlockComment:
                    flags |= TokenFlag::AfterWhitespace;
                    continue;

                // We don't want to skip the end-of-file token, but we *do*
                // want to make sure it has appropriate flags to make our life easier
                case TokenType::EndOfFile:
                    flags = TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace;
                    break;

                // We will also do some book-keeping around preprocessor directives here:
                //
                // If we see a `#` at the start of a line, then we are entering a
                // preprocessor directive.
                case TokenType::Pound:
                    if((flags & TokenFlag::AtStartOfLine) != 0)
                        lexerFlags |= kLexerFlag_InDirective;
                    break;
                //
                // And if we saw an end-of-line during a directive, then we are
                // now leaving that directive.
                //
                case TokenType::EndOfDirective:
                    lexerFlags &= ~kLexerFlag_InDirective;
                    break;

                default:
                    break;
                }

                token.Type =  tokenType;

                char const* textEnd = cursor;

                // Note(tfoley): `StringBuilder::Append()` seems to crash when appending zero bytes
                if(textEnd != textBegin)
                {
                    StringBuilder valueBuilder;
                    valueBuilder.Append(textBegin, int(textEnd - textBegin));
                    token.Content = valueBuilder.ProduceString();
                }

                token.flags = flags;

                this->tokenFlags = 0;

                return token;
            }
        }

        TokenList Lexer::lexAllTokens()
        {
            TokenList tokenList;
            for(;;)
            {
                Token token = lexToken();
                tokenList.mTokens.Add(token);

                if(token.Type == TokenType::EndOfFile)
                    return tokenList;
            }
        }



#if 0
        TokenList Lexer::Parse(const String & fileName, const String & str, DiagnosticSink * sink)
        {
            TokenList tokenList;
            tokenList.mTokens = TokenizeText(fileName, str, [&](TokenizeErrorType errType, CodePosition pos)
            {
                auto curChar = str[pos.Pos];
                switch (errType)
                {
                case TokenizeErrorType::InvalidCharacter:
                    // Check if inside the ASCII "printable" range
                    if(curChar >= 0x20 && curChar <=  0x7E)
                    {
                        char buffer[] = { curChar, 0 };
                        sink->diagnose(pos, Diagnostics::illegalCharacterPrint, buffer);
                    }
                    else
                    {
                        // Fallback: print as hexadecimal
                        sink->diagnose(pos, Diagnostics::illegalCharacterHex, String((unsigned char)curChar, 16));
                    }
                    break;
                case TokenizeErrorType::InvalidEscapeSequence:
                    sink->diagnose(pos, Diagnostics::illegalCharacterLiteral);
                    break;
                default:
                    break;
                }
            });

            // Add an end-of-file token so that we can reference it in diagnostic messages
            tokenList.mTokens.Add(Token(TokenType::EndOfFile, "", 0, 0, 0, fileName, TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace));
            return tokenList;
        }
#endif
    }
}