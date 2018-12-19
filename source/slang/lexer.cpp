// lexer.cpp
#include "lexer.h"

// This file implements the lexer/scanner, which is responsible for taking a raw stream of
// input bytes and turning it into semantically useful tokens.
//

#include "compiler.h"
#include "source-loc.h"

#include <assert.h>

namespace Slang
{
    static Token GetEndOfFileToken()
    {
        return Token(TokenType::EndOfFile, UnownedStringSlice::fromLiteral(""), SourceLoc());
    }

    Token* TokenList::begin() const
    {
        SLANG_ASSERT(mTokens.Count());
        return &mTokens[0];
    }

    Token* TokenList::end() const
    {
        SLANG_ASSERT(mTokens.Count());
        SLANG_ASSERT(mTokens[mTokens.Count()-1].type == TokenType::EndOfFile);
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
            token.type = TokenType::EndOfFile;
        return token;
    }

    TokenType TokenReader::PeekTokenType() const
    {
        if (mCursor == mEnd)
            return TokenType::EndOfFile;
        SLANG_ASSERT(mCursor);
        return mCursor->type;
    }

    SourceLoc TokenReader::PeekLoc() const
    {
        if (!mCursor)
            return SourceLoc();
        SLANG_ASSERT(mCursor);
        return mCursor->loc;
    }

    Token TokenReader::AdvanceToken()
    {
        if (!mCursor)
            return GetEndOfFileToken();

        Token token = *mCursor;
        if (mCursor == mEnd)
            token.type = TokenType::EndOfFile;
        else
            mCursor++;
        return token;
    }

    // Lexer

    void Lexer::initialize(
        SourceView*     inSourceView,
        DiagnosticSink* inSink,
        NamePool*       inNamePool,
        MemoryArena*    inMemoryArena)
    {
        sourceView  = inSourceView;
        sink        = inSink;
        namePool    = inNamePool;
        memoryArena = inMemoryArena;

        auto content = inSourceView->getContent();
        
        begin   = content.begin();
        cursor  = content.begin();
        end     = content.end();

        // Set the start location
        startLoc = inSourceView->getRange().begin;

        tokenFlags = TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace;
        lexerFlags = 0;
    }

    Lexer::~Lexer()
    {
    }

    enum { kEOF = -1 };

    // Get the next input byte, without any handling of
    // escaped newlines, non-ASCII code points, source locations, etc.
    static int peekRaw(Lexer* lexer)
    {
        // If we are at the end of the input, return a designated end-of-file value
        if(lexer->cursor == lexer->end)
            return kEOF;

        // Otherwise, just look at the next byte
        return *lexer->cursor;
    }

    // Read one input byte without any special handling (similar to `peekRaw`)
    static int advanceRaw(Lexer* lexer)
    {
        // The logic here is basically the same as for `peekRaw()`,
        // escape we advance `cursor` if we aren't at the end.

        if (lexer->cursor == lexer->end)
            return kEOF;

        return *lexer->cursor++;
    }

    // When the cursor is already at the first byte of an end-of-line sequence,
    // consume one or two bytes that compose the sequence.
    //
    // Basically, a newline is one of:
    //
    //  "\n"
    //  "\r"
    //  "\r\n"
    //  "\n\r"
    //
    // We always look for the longest match possible.
    //
    static void handleNewLineInner(Lexer* lexer, int c)
    {
        SLANG_ASSERT(c == '\n' || c == '\r');

        int d = peekRaw(lexer);
        if( (c ^ d) == ('\n' ^ '\r') )
        {
            advanceRaw(lexer);
        }
    }

    // Look ahead one code point, dealing with complications like
    // escaped newlines.
    static int peek(Lexer* lexer)
    {
        // Look at the next raw byte, and decide what to do
        int c = peekRaw(lexer);

        if(c == '\\')
        {
            // We might have a backslash-escaped newline.
            // Look at the next byte (if any) to see.
            //
            // Note(tfoley): We are assuming a null-terminated input here,
            // so that we can safely look at the next byte without issue.
            int d = lexer->cursor[1];
            switch (d)
            {
            case '\r': case '\n':
                {
                    // The newline was escaped, so return the code point after *that*

                    int e = lexer->cursor[2];
                    if ((d ^ e) == ('\r' ^ '\n'))
                        return lexer->cursor[3];
                    return e;
                }

            default:
                break;
            }
        }
        // TODO: handle UTF-8 encoding for non-ASCII code points here

        // Default case is to just hand along the byte we read as an ASCII code point.
        return c;
    }

    // Get the next code point from the input, and advance the cursor.
    static int advance(Lexer* lexer)
    {
        // We are going to loop, but only as a way of handling
        // escaped line endings.
        for (;;)
        {
            // If we are at the end of the input, then the task is easy.
            if (lexer->cursor == lexer->end)
                return kEOF;

            // Look at the next raw byte, and decide what to do
            int c = *lexer->cursor++;

            if (c == '\\')
            {
                // We might have a backslash-escaped newline.
                // Look at the next byte (if any) to see.
                //
                // Note(tfoley): We are assuming a null-terminated input here,
                // so that we can safely look at the next byte without issue.
                int d = *lexer->cursor;
                switch (d)
                {
                case '\r': case '\n':
                    // handle the end-of-line for our source location tracking
                    lexer->cursor++;
                    handleNewLineInner(lexer, d);

                    lexer->tokenFlags |= TokenFlag::ScrubbingNeeded;

                    // Now try again, looking at the character after the
                    // escaped newline.
                    continue;

                default:
                    break;
                }
            }

            // TODO: Need to handle non-ASCII code points.

            // Default case is to return the raw byte we saw.
            return c;
        }
    }

    static void handleNewLine(Lexer* lexer)
    {
        int c = advance(lexer);
        handleNewLineInner(lexer, c);
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

    static SourceLoc getSourceLoc(Lexer* lexer)
    {
        return lexer->startLoc + (lexer->cursor - lexer->begin);
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
                lexer->sink->diagnose(getSourceLoc(lexer), Diagnostics::invalidDigitForBase, buffer, base);
            }

            advance(lexer);
        }
    }

    static TokenType maybeLexNumberSuffix(Lexer* lexer, TokenType tokenType)
    {
        // Be liberal in what we accept here, so that figuring out
        // the semantics of a numeric suffix is left up to the parser
        // and semantic checking logic.
        //
        for( ;;)
        {
            int c = peek(lexer);

            // Accept any alphanumeric character, plus underscores.
            if(('a' <= c ) && (c <= 'z')
                || ('A' <= c) && (c <= 'Z')
                || ('0' <= c) && (c <= '9')
                || (c == '_'))
            {
                advance(lexer);
                continue;
            }

            // Stop at the first character that isn't
            // alphanumeric.
            return tokenType;
        }
    }

    static bool isNumberExponent(int c, int base)
    {
        switch( c )
        {
        default:
            return false;

        case 'e': case 'E':
            if(base != 10) return false;
            break;

        case 'p': case 'P':
            if(base != 16) return false;
            break;
        }

        return true;
    }

    static bool maybeLexNumberExponent(Lexer* lexer, int base)
    {
        if(!isNumberExponent(peek(lexer), base))
            return false;

        // we saw an exponent marker
        advance(lexer);

        // Now start to read the exponent
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

        return maybeLexNumberSuffix(lexer, TokenType::FloatingPointLiteral);
    }

    static TokenType lexNumber(Lexer* lexer, int base)
    {
        // TODO(tfoley): Need to consider whehter to allow any kind of digit separator character.

        TokenType tokenType = TokenType::IntegerLiteral;

        // At the start of things, we just concern ourselves with digits
        lexDigits(lexer, base);

        if( peek(lexer) == '.' )
        {
            tokenType = TokenType::FloatingPointLiteral;

            advance(lexer);
            lexDigits(lexer, base);
        }

        if( maybeLexNumberExponent(lexer, base))
        {
            tokenType = TokenType::FloatingPointLiteral;
        }

        maybeLexNumberSuffix(lexer, tokenType);
        return tokenType;
    }

    static int maybeReadDigit(char const** ioCursor, int base)
    {
        auto& cursor = *ioCursor;

        for(;;)
        {
            int c = *cursor;
            switch(c)
            {
            default:
                return -1;

            // TODO: need to decide on digit separator characters
            case '_':
                cursor++;
                continue;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                cursor++;
                return c - '0';

            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                if(base > 10)
                {
                    cursor++;
                    return 10 + c - 'a';
                }
                return -1;

            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                if(base > 10)
                {
                    cursor++;
                    return 10 + c - 'A';
                }
                return -1;
            }
        }
    }

    static int readOptionalBase(char const** ioCursor)
    {
        auto& cursor = *ioCursor;
        if( *cursor == '0' )
        {
            cursor++;
            switch(*cursor)
            {
            case 'x': case 'X':
                cursor++;
                return 16;

            case 'b': case 'B':
                cursor++;
                return 2;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return 8;

            default:
                return 10;
            }
        }

        return 10;
    }



    IntegerLiteralValue getIntegerLiteralValue(Token const& token, UnownedStringSlice* outSuffix)
    {
        IntegerLiteralValue value = 0;

        char const* cursor = token.Content.begin();
        char const* end = token.Content.end();

        int base = readOptionalBase(&cursor);

        for( ;;)
        {
            int digit = maybeReadDigit(&cursor, base);
            if(digit < 0)
                break;

            value = value*base + digit;
        }

        if(outSuffix)
        {
            *outSuffix = UnownedStringSlice(cursor, end);
        }

        return value;
    }

    FloatingPointLiteralValue getFloatingPointLiteralValue(Token const& token, UnownedStringSlice* outSuffix)
    {
        FloatingPointLiteralValue value = 0;

        char const* cursor = token.Content.begin();
        char const* end = token.Content.end();

        int radix = readOptionalBase(&cursor);

        bool seenDot = false;
        FloatingPointLiteralValue divisor = 1;
        for( ;;)
        {
            if(*cursor == '.')
            {
                cursor++;
                seenDot = true;
                continue;
            }

            int digit = maybeReadDigit(&cursor, radix);
            if(digit < 0)
                break;

            value = value*radix + digit;

            if(seenDot)
            {
                divisor *= radix;
            }
        }

        // Now read optional exponent
        if(isNumberExponent(*cursor, radix))
        {
            cursor++;

            bool exponentIsNegative = false;
            switch(*cursor)
            {
            default:
                break;

            case '-':
                exponentIsNegative = true;
                cursor++;
                break;

            case '+':
                cursor++;
                break;
            }

            int exponentRadix = 10;
            int exponent = 0;

            for(;;)
            {
                int digit = maybeReadDigit(&cursor, exponentRadix);
                if(digit < 0)
                    break;

                exponent = exponent*exponentRadix + digit;
            }

            FloatingPointLiteralValue exponentBase = 10;
            if(radix == 16)
            {
                exponentBase = 2;
            }

            FloatingPointLiteralValue exponentValue = pow(exponentBase, exponent);

            if( exponentIsNegative )
            {
                divisor *= exponentValue;
            }
            else
            {
                value *= exponentValue;
            }
        }

        value /= divisor;

        if(outSuffix)
        {
            *outSuffix = UnownedStringSlice(cursor, end);
        }

        return value;
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
                lexer->sink->diagnose(getSourceLoc(lexer), Diagnostics::endOfFileInLiteral);
                return;

            case '\n': case '\r':
                lexer->sink->diagnose(getSourceLoc(lexer), Diagnostics::newlineInLiteral);
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
        SLANG_ASSERT(token.type == TokenType::StringLiteral
            || token.type == TokenType::CharLiteral);

        char const* cursor = token.Content.begin();
        char const* end = token.Content.end();
        SLANG_UNREFERENCED_VARIABLE(end);

        auto quote = *cursor++;
        SLANG_ASSERT(quote == '\'' || quote == '"');

        StringBuilder valueBuilder;
        for(;;)
        {
            SLANG_ASSERT(cursor != end);

            auto c = *cursor++;

            // If we see a closing quote, then we are at the end of the string literal
            if(c == quote)
            {
                SLANG_ASSERT(cursor == end);
                return valueBuilder.ProduceString();
            }

            // Characters that don't being escape sequences are easy;
            // just append them to the buffer and move on.
            if(c != '\\')
            {
                valueBuilder.Append(c);
                continue;
            }

            // Now we look at another character to figure out the kind of
            // escape sequence we are dealing with:

            char d = *cursor++;

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
        // path separator character).

        // Just trim off the first and last characters to remove the quotes
        // (whether they were `""` or `<>`.
        return String(token.Content.begin() + 1, token.Content.end() - 1); 
    }



    static TokenType lexTokenImpl(Lexer* lexer, LexerFlags effectiveFlags)
    {
        if(effectiveFlags & kLexerFlag_ExpectDirectiveMessage)
        {
            for(;;)
            {
                switch(peek(lexer))
                {
                default:
                    advance(lexer);
                    continue;

                case kEOF: case '\r': case '\n':
                    break;
                }
                break;
            }
            return TokenType::DirectiveMessage;
        }

        switch(peek(lexer))
        {
        default:
            break;

        case kEOF:
            if((effectiveFlags & kLexerFlag_InDirective) != 0)
                return TokenType::EndOfDirective;
            return TokenType::EndOfFile;

        case '\r': case '\n':
            if((effectiveFlags & kLexerFlag_InDirective) != 0)
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
                auto loc = getSourceLoc(lexer);
                advance(lexer);
                switch(peek(lexer))
                {
                default:
                    return maybeLexNumberSuffix(lexer, TokenType::IntegerLiteral);

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
            return TokenType::StringLiteral;

        case '\'':
            advance(lexer);
            lexStringLiteralBody(lexer, '\'');
            return TokenType::CharLiteral;

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

        case ':': 
        {
            advance(lexer);
            if (peek(lexer) == ':')
            {
                advance(lexer);
                return TokenType::Scope;
            }
            return TokenType::Colon;
        }
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

            auto loc = getSourceLoc(lexer);
            int c = advance(lexer);
            if(!(effectiveFlags & kLexerFlag_IgnoreInvalid))
            {
                auto sink = lexer->sink;
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
            }

            return TokenType::Invalid;
        }
    }

    Token Lexer::lexToken(LexerFlags extraFlags)
    {
        auto& flags = this->tokenFlags;
        for(;;)
        {
            Token token;
            token.loc = getSourceLoc(this);

            char const* textBegin = cursor;

            auto tokenType = lexTokenImpl(this, this->lexerFlags | extraFlags);

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
                flags |= TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace;
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

            token.type =  tokenType;

            char const* textEnd = cursor;

            // Note(tfoley): `StringBuilder::Append()` seems to crash when appending zero bytes
            if(textEnd != textBegin)
            {
                // "scrubbing" token value here to remove escaped newlines...
                //
                // Only perform this work if we encountered an escaped newline
                // while lexing this token (e.g., keep a flag on the lexer), or
                // do it on-demand when the actual value of the token is needed.
                if (tokenFlags & TokenFlag::ScrubbingNeeded)
                {
                    // Allocate space that will always be more than enough for stripped contents
                    char* startDst = (char*)memoryArena->allocateUnaligned(textEnd - textBegin);
                    char* dst = startDst;

                    auto tt = textBegin;
                    while (tt != textEnd)
                    {
                        char c = *tt++;
                        if (c == '\\')
                        {
                            char d = *tt;
                            switch (d)
                            {
                            case '\r': case '\n':
                            {
                                tt++;
                                char e = *tt;
                                if ((d ^ e) == ('\r' ^ '\n'))
                                {
                                    tt++;
                                }
                            }
                            continue;

                            default:
                                break;
                            }
                        }
                        *dst++ = c;
                    }
                    token.Content = UnownedStringSlice(startDst, dst);
                }
                else
                {
                    token.Content = UnownedStringSlice(textBegin, textEnd);
                }
            }

            token.flags = flags;

            this->tokenFlags = 0;

            if (tokenType == TokenType::Identifier)
            {
                token.ptrValue = this->namePool->getName(token.Content);
            }

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

            if(token.type == TokenType::EndOfFile)
                return tokenList;
        }
    }
}
