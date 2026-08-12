// slang-lexer.cpp
#include "slang-lexer.h"

// This file implements the lexer/scanner, which is responsible for taking a raw stream of
// input bytes and turning it into semantically useful tokens.
//

#include "core/slang-char-encode.h"
#include "core/slang-string-escape-util.h"
#include "core/slang-string-util.h"
#include "fast_float/fast_float.h"
#include "slang-core-diagnostics.h"
#include "slang-name.h"
#include "slang-source-loc.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

namespace Slang
{
Token TokenReader::getEndOfFileToken()
{
    return Token(TokenType::EndOfFile, UnownedStringSlice::fromLiteral(""), SourceLoc());
}

const Token* TokenList::begin() const
{
    SLANG_ASSERT(m_tokens.getCount());
    return &m_tokens[0];
}

const Token* TokenList::end() const
{
    SLANG_ASSERT(m_tokens.getCount());
    SLANG_ASSERT(m_tokens[m_tokens.getCount() - 1].type == TokenType::EndOfFile);
    return &m_tokens[m_tokens.getCount() - 1];
}

TokenSpan::TokenSpan()
    : m_begin(nullptr), m_end(nullptr)
{
}

TokenReader::TokenReader()
    : m_cursor(nullptr), m_end(nullptr)
{
    _updateLookaheadToken();
}

Token& TokenReader::peekToken()
{
    return m_nextToken;
}

TokenType TokenReader::peekTokenType() const
{
    return m_nextToken.type;
}

SourceLoc TokenReader::peekLoc() const
{
    return m_nextToken.loc;
}

Token TokenReader::advanceToken()
{
    Token result = m_nextToken;
    if (m_cursor != m_end)
        m_cursor++;
    _updateLookaheadToken();
    return result;
}

void TokenReader::_updateLookaheadToken()
{
    // We assume here that we can read a token from a non-null `m_cursor`
    // *even* in the case where `m_cursor == m_end`, because the invariant
    // for lists of tokens is that they should be terminated with and
    // end-of-file token, so that there is always a token "one past the end."
    //
    m_nextToken = m_cursor ? *m_cursor : getEndOfFileToken();

    // If the token we read came from the end of the sub-sequence we are
    // reading, then we will change the token type to an end-of-file token
    // so that code that reads from the sequence and expects a terminating
    // EOF will find it.
    //
    // TODO: We might eventually want a way to look at the actual token type
    // and not just use EOF in all cases: e.g., when emitting diagnostic
    // messages that include the token that is seen.
    //
    if (m_cursor == m_end)
        m_nextToken.type = TokenType::EndOfFile;
}

// Lexer

void Lexer::initialize(
    SourceView* sourceView,
    DiagnosticSink* sink,
    NamePool* namePool,
    MemoryArena* memoryArena)
{
    m_sourceView = sourceView;
    m_sink = sink;
    m_namePool = namePool;
    m_memoryArena = memoryArena;

    auto content = sourceView->getContent();

    m_begin = content.begin();
    m_cursor = content.begin();
    m_end = content.end();

    // Set the start location
    m_startLoc = sourceView->getRange().begin;

    // The first token read from a translation unit should be considered to be at
    // the start of a line, and *also* as coming after whitespace (conceptually
    // both the end-of-file and beginning-of-file pseudo-tokens are whitespace).
    //
    m_tokenFlags = TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace;
    m_lexerFlags = 0;
}

Lexer::~Lexer() {}

enum
{
    kEOF = -1
};

static const int kMaxLexErrorCount = 100;

template<typename P, typename... Args>
static void diagnose(
    DiagnosticSink* sink,
    const P& loc,
    const DiagnosticInfo& info,
    const Args&... args)
{
    if (!sink)
        return;

    // Cap max errors to avoid flooding the sink memory.
    if (sink->getErrorCount() > kMaxLexErrorCount)
        return;
    sink->diagnose(loc, info, args...);
}

// Get the next input byte, without any handling of
// escaped newlines, non-ASCII code points, source locations, etc.
static int _peekRaw(Lexer* lexer)
{
    // If we are at the end of the input, return a designated end-of-file value
    if (lexer->m_cursor == lexer->m_end)
        return kEOF;

    // Otherwise, just look at the next byte
    return *lexer->m_cursor;
}

// Read one input byte without any special handling (similar to `peekRaw`)
static int _advanceRaw(Lexer* lexer)
{
    // The logic here is basically the same as for `peekRaw()`,
    // escape we advance `cursor` if we aren't at the end.

    if (lexer->m_cursor == lexer->m_end)
        return kEOF;

    return *lexer->m_cursor++;
}

static SourceLoc _getSourceLoc(const Lexer& lexer, const char* it)
{
    return lexer.m_startLoc + (it - lexer.m_begin);
}

static SourceLoc _getSourceLoc(const Lexer* lexer)
{
    return _getSourceLoc(*lexer, lexer->m_cursor);
}

static String _inputToByteSeq(const char* b, const char* e)
{
    StringBuilder byteSeqBuilder;
    for (const char* i = b; i != e; ++i)
    {
        StringUtil::appendFormat(
            byteSeqBuilder,
            (i == b) ? "0x%02X" : " 0x%02X",
            unsigned{static_cast<unsigned char>(*i)});
    }
    return byteSeqBuilder.toString();
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
static void _handleNewLineInner(Lexer* lexer, int c)
{
    SLANG_ASSERT(c == '\n' || c == '\r');

    int d = _peekRaw(lexer);
    if ((c ^ d) == ('\n' ^ '\r'))
    {
        _advanceRaw(lexer);
    }
}

// Look ahead one code point, dealing with complications like
// escaped newlines.
static int _peek(Lexer* lexer, int offset = 0)
{
    int pos = 0;
    int c = kEOF;

    do
    {
        if (lexer->m_cursor + pos >= lexer->m_end)
            return kEOF;

        c = lexer->m_cursor[pos++];

        while (c == '\\')
        {
            // We might have a backslash-escaped newline.
            // Look at the next byte (if any) to see.
            //
            // Note(tfoley): We are assuming a null-terminated input here,
            // so that we can safely look at the next byte without issue.
            int d = lexer->m_cursor[pos++];
            switch (d)
            {
            case '\r':
            case '\n':
                {
                    // The newline was escaped, so return the code point after *that*
                    int e = lexer->m_cursor[pos++];
                    if ((d ^ e) == ('\r' ^ '\n'))
                        c = lexer->m_cursor[pos++];
                    else
                        c = e;
                    continue;
                }
            default:
                break;
            }

            // Only continue this while loop in the case where we consumed
            // some newlines
            break;
        }
        if (isUtf8LeadingByte((Byte)c))
        {
            // Consume all unicode characters.
            pos--;
            bool first{true};
            bool invalid{};
            c = getUnicodePointFromUTF8(
                [&]() -> unsigned char
                {
                    if (lexer->m_cursor + pos >= lexer->m_end)
                        return 0U;

                    if (first || isUtf8ContinuationByte(lexer->m_cursor[pos]))
                    {
                        first = false;
                        return static_cast<unsigned char>(lexer->m_cursor[pos++]);
                    }

                    // Current byte is not a continuation byte, so we don't
                    // consume it. Instead, we'll just return a poison byte to
                    // ensure that the current sequence is interpreted as
                    // invalid.
                    return 0xFFU; // always invalid UTF-8
                },
                &invalid);

            // If the UTF-8 sequence is invalid, we'll return space,
            // instead. This will be diagnosed in _advance().
            if (invalid)
                c = ' ';
        }
        // Default case is to just hand along the byte we read as an ASCII code point.
    } while (offset--);

    // If we encounter a \0, return kEOF.
    // if (c == 0)
    //    return kEOF;
    return c;
}

// Get the next code point from the input, and advance the cursor.
static int _advance(Lexer* lexer)
{
    // We are going to loop, but only as a way of handling
    // escaped line endings.
    for (;;)
    {
        // If we are at the end of the input, then the task is easy.
        if (lexer->m_cursor >= lexer->m_end)
            return kEOF;

        // Look at the next raw byte, and decide what to do
        int c = *lexer->m_cursor++;

        if (c == '\\')
        {
            // We might have a backslash-escaped newline.
            // Look at the next byte (if any) to see.
            //
            // Note(tfoley): We are assuming a null-terminated input here,
            // so that we can safely look at the next byte without issue.
            int d = *lexer->m_cursor;
            switch (d)
            {
            case '\r':
            case '\n':
                // handle the end-of-line for our source location tracking
                lexer->m_cursor++;
                _handleNewLineInner(lexer, d);

                lexer->m_tokenFlags |= TokenFlag::ScrubbingNeeded;

                // Now try again, looking at the character after the
                // escaped newline.
                continue;

            default:
                break;
            }
        }

        // Consume all unicode characters.
        bool isInvalidStream = false;
        if (isUtf8LeadingByte((Byte)c))
        {
            lexer->m_cursor--;
            const char* seqStart = lexer->m_cursor;
            bool invalid{};
            c = getUnicodePointFromUTF8(
                [&]() -> unsigned char
                {
                    if (lexer->m_cursor >= lexer->m_end)
                    {
                        isInvalidStream = true;
                        return 0U;
                    }

                    if ((lexer->m_cursor == seqStart) || isUtf8ContinuationByte(*lexer->m_cursor))
                        return static_cast<unsigned char>(*lexer->m_cursor++);

                    // Current byte is not a continuation byte, so we don't
                    // consume it. Instead, we'll just return a poison byte to
                    // ensure that the current sequence is interpreted as
                    // invalid.
                    return 0xFFU; // always invalid UTF-8
                },
                &invalid);

            // If the UTF-8 sequence is invalid, we'll interpret it as a space
            // and diagnose.
            if (invalid)
            {
                diagnose(
                    lexer->m_sink,
                    _getSourceLoc(*lexer, seqStart),
                    LexerDiagnostics::invalidUtf8ByteSequence,
                    _inputToByteSeq(seqStart, lexer->m_cursor).getBuffer());
                c = ' ';
            }
        }

        // If we encounter a \0, return kEOF, and move stream cursor to the end.
        if (c == 0 || isInvalidStream)
        {
            lexer->m_cursor = lexer->m_end;
        }

        // Default case is to return the raw byte we saw.
        return c;
    }
}

static void _handleNewLine(Lexer* lexer)
{
    int c = _advance(lexer);
    _handleNewLineInner(lexer, c);
}

static void _lexLineComment(Lexer* lexer)
{
    for (;;)
    {
        switch (_peek(lexer))
        {
        case '\n':
        case '\r':
        case kEOF:
            return;

        default:
            _advance(lexer);
            continue;
        }
    }
}

static void _lexBlockComment(Lexer* lexer)
{
    for (;;)
    {
        switch (_peek(lexer))
        {
        case kEOF:
            // TODO(tfoley) diagnostic!
            return;

        case '\n':
        case '\r':
            _handleNewLine(lexer);
            continue;

        case '*':
            _advance(lexer);
            switch (_peek(lexer))
            {
            case '/':
                _advance(lexer);
                return;

            default:
                continue;
            }

        default:
            _advance(lexer);
            continue;
        }
    }
}

static void _lexHorizontalSpace(Lexer* lexer)
{
    for (;;)
    {
        switch (_peek(lexer))
        {
        case ' ':
        case '\t':
            _advance(lexer);
            continue;

        default:
            return;
        }
    }
}

static bool isNonAsciiCodePoint(unsigned int codePoint)
{
    return codePoint != 0xFFFFFFFF && codePoint >= 0x80;
}

static void _lexIdentifier(Lexer* lexer)
{
    for (;;)
    {
        int c = _peek(lexer);
        if (('a' <= c) && (c <= 'z') || ('A' <= c) && (c <= 'Z') || ('0' <= c) && (c <= '9') ||
            (c == '_') || isNonAsciiCodePoint((unsigned int)c))
        {
            _advance(lexer);
            continue;
        }
        return;
    }
}

static void _lexDigits(Lexer* lexer, int base)
{
    for (;;)
    {
        int c = _peek(lexer);

        int digitVal = 0;
        switch (c)
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            digitVal = c - '0';
            break;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            if (base <= 10)
                return;
            digitVal = 10 + c - 'a';
            break;

        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            if (base <= 10)
                return;
            digitVal = 10 + c - 'A';
            break;

        default:
            // Not more digits!
            return;
        }

        if (digitVal >= base)
        {
            if (auto sink = lexer->getDiagnosticSink())
            {
                char buffer[] = {(char)c, 0};
                diagnose(
                    sink,
                    _getSourceLoc(lexer),
                    LexerDiagnostics::invalidDigitForBase,
                    buffer,
                    base);
            }
        }

        _advance(lexer);
    }
}

static TokenType _maybeLexNumberSuffix(Lexer* lexer, TokenType tokenType)
{
    // Be liberal in what we accept here, so that figuring out
    // the semantics of a numeric suffix is left up to the parser
    // and semantic checking logic.
    //
    for (;;)
    {
        int c = _peek(lexer);

        // Accept any alphanumeric character, plus underscores.
        if (('a' <= c) && (c <= 'z') || ('A' <= c) && (c <= 'Z') || ('0' <= c) && (c <= '9') ||
            (c == '_'))
        {
            _advance(lexer);
            continue;
        }

        // Stop at the first character that isn't
        // alphanumeric.
        return tokenType;
    }
}

static bool _isNumberExponent(int c, int base)
{
    switch (c)
    {
    default:
        return false;

    case 'e':
    case 'E':
        if (base != 10)
            return false;
        break;

    case 'p':
    case 'P':
        if (base != 16)
            return false;
        break;
    }

    return true;
}

static bool _maybeLexNumberExponent(Lexer* lexer, int base)
{
    if (_peek(lexer) == '#')
    {
        // Special case #INF
        const auto inf = toSlice("#INF");
        for (auto c : inf)
        {
            if (_peek(lexer) != c)
            {
                return false;
            }
            _advance(lexer);
        }

        return true;
    }

    if (!_isNumberExponent(_peek(lexer), base))
        return false;

    // we saw an exponent marker
    _advance(lexer);

    // Now start to read the exponent
    switch (_peek(lexer))
    {
    case '+':
    case '-':
        _advance(lexer);
        break;
    }

    // TODO(tfoley): it would be an error to not see digits here...

    _lexDigits(lexer, 10);

    return true;
}

static TokenType _lexNumberAfterDecimalPoint(Lexer* lexer, int base)
{
    _lexDigits(lexer, base);
    _maybeLexNumberExponent(lexer, base);

    return _maybeLexNumberSuffix(lexer, TokenType::FloatingPointLiteral);
}

static TokenType _lexNumber(Lexer* lexer, int base)
{
    // TODO(tfoley): Need to consider whether to allow any kind of digit separator character.

    TokenType tokenType = TokenType::IntegerLiteral;

    // At the start of things, we just concern ourselves with digits
    _lexDigits(lexer, base);

    if (_peek(lexer) == '.')
    {
        switch (_peek(lexer, 1))
        {
            // 123.xxxx or 123.rrrr
        case 'x':
        case 'r':
            break;

        default:
            tokenType = TokenType::FloatingPointLiteral;

            _advance(lexer);
            _lexDigits(lexer, base);
        }
    }

    if (_maybeLexNumberExponent(lexer, base))
    {
        tokenType = TokenType::FloatingPointLiteral;
    }

    _maybeLexNumberSuffix(lexer, tokenType);
    return tokenType;
}

static int _maybeReadDigit(char const** ioCursor, int base)
{
    auto& cursor = *ioCursor;

    for (;;)
    {
        int c = *cursor;
        switch (c)
        {
        default:
            return -1;

        // TODO: need to decide on digit separator characters
        case '_':
            cursor++;
            continue;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            cursor++;
            return c - '0';

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            if (base > 10)
            {
                cursor++;
                return 10 + c - 'a';
            }
            return -1;

        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            if (base > 10)
            {
                cursor++;
                return 10 + c - 'A';
            }
            return -1;
        }
    }
}

static int _readOptionalBase(char const** ioCursor)
{
    auto& cursor = *ioCursor;
    if (*cursor == '0')
    {
        cursor++;
        switch (*cursor)
        {
        case 'x':
        case 'X':
            cursor++;
            return 16;

        case 'b':
        case 'B':
            cursor++;
            return 2;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return 8;

        default:
            return 10;
        }
    }

    return 10;
}


IntegerLiteralValue getIntegerLiteralValue(
    Token const& token,
    DiagnosticSink* sink,
    UnownedStringSlice* outSuffix,
    bool* outIsDecimalBase,
    bool* outHasOverflowed)
{
    uint64_t value = 0;

    const UnownedStringSlice content = token.getContent();

    char const* cursor = content.begin();
    char const* end = content.end();

    int base = _readOptionalBase(&cursor);
    bool hasOverflowed = false;

    for (;;)
    {
        int digit = _maybeReadDigit(&cursor, base);
        if (digit < 0)
            break;

        if (value > (UINT64_MAX - digit) / base)
            hasOverflowed = true;
        value = value * base + digit;
    }

    if (hasOverflowed)
    {
        diagnose(sink, token.getLoc(), LexerDiagnostics::integerLiteralTooLargeForAnyType);
    }

    if (outSuffix)
    {
        *outSuffix = UnownedStringSlice(cursor, end);
    }

    if (outIsDecimalBase)
    {
        *outIsDecimalBase = (base == 10);
    }

    if (outHasOverflowed)
    {
        *outHasOverflowed = hasOverflowed;
    }

    return value;
}

// Rounds a non-negative double to the precision and range of a narrower float
// type, returning the result still held in a double.
//
// This function assumes that 'value' is non-negative.
//
// Parameters:
//
//   value           - Value to round. When a regular number, must be >= 0. May also be 0,
//                     +infinity, NaN.
//   minNormalExp    - Minimum normal exponent before subnormal
//   maxExp          - Maximum exponent. Anything above that is +INFINITY
//   precisionBits   - Precision in number of bits
//   roundToNearest  - Rounding mode: true = round to nearest, ties to even. False = truncate
//                     (round towards zero)
//
// Note:
// - float:  -126, +127, 24
// - half:   -14,  +15,  11
static double _truncateDouble(
    double value,
    int minNormalExp,
    int maxExp,
    unsigned precisionBits,
    bool roundToNearest)
{
    // NaNs and INFs are passed as is
    if (!std::isfinite(value))
        return value;

    SLANG_ASSERT(value >= 0.0);

    // first check for overflow
    if (roundToNearest)
    {
        // anything at or above the tie point that rounds up to maxExp+1 overflows
        const double limit =
            std::ldexp(2.0, maxExp) - std::ldexp(1.0, maxExp - static_cast<int>(precisionBits));
        if (value >= limit)
            return std::numeric_limits<double>::infinity();
    }
    else
    {
        const double limit = std::ldexp(2.0, maxExp);
        if (value >= limit)
            return std::numeric_limits<double>::infinity();
    }

    // Note: there is a seeming off-by-one with exponents. This is because
    // frexp() returns a fraction between [0.5, 1). That is, a number such as
    // 3.5 is decomposed as 0.875 * 2^2, instead of 1.75 * 2^1.
    int exp{};
    double fraction = std::frexp(value, &exp);

    // Additional precision reduction for subnormals - note the exponent
    // off-by-one comment above.
    int precisionLoss = std::max(minNormalExp - (exp - 1), 0);

    int exponentShift = static_cast<int>(precisionBits) - precisionLoss;

    // scale the fraction so that the retained bits become the integer part
    fraction = std::ldexp(fraction, exponentShift);

    if (roundToNearest)
    {
        // To make the rounding precise, we need to divide the number into integer and fractional
        // parts
        double integerPart{};

        // yyyyyyy.xxxxxxxx
        // integer.roundoff

        double roundOffPart = std::modf(fraction, &integerPart);
        if (roundOffPart != 0.5)
        {
            fraction = std::round(fraction);
        }
        else
        {
            // Tied. The tie breaker is the least significant retained
            // bit. Round up or down to make it 0.

            // integerPart / 2:
            //
            // intege.r00000
            //        |
            //        \- The least significant retained bit. Note: Round-off part
            //           is 0.5 since we're in this branch.

            [[maybe_unused]] double unused{};
            double lsb = std::modf(integerPart / 2.0, &unused);
            if (lsb >= 0.5)
                fraction = std::round(fraction); // round up to make integerPart even
            else
                fraction = std::trunc(fraction); // round down to keep integerPart even
        }
    }
    else
    {
        // simple truncation
        fraction = std::trunc(fraction);
    }

    fraction = std::ldexp(fraction, -exponentShift);

    // return rounded double
    return std::ldexp(fraction, exp);
}

// Converts a literal in hexadecimal format to double. The return value is truncated
// in case the significand in the literal cannot be fit.
//
// Params:
//
//   start            - literal (after 0x)
//   end              - end of literal; start + strlen(literal)
//   suffix           - pointer to receive the first unhandled character
//   outIsOutOfRange  - Whether the value is out of range. Return value is
//                      either 0 for subnormal underflow or INFINITY for overflow.
//   outPrecisionLost - Significand was truncated. Only reported if the value was in range.
//
// Return:              Parsed value.
//
//
static double _hexFloatLiteralToDouble(
    const char* start,
    const char* end,
    const char*& suffix,
    bool& outIsOutOfRange,
    bool& outPrecisionLost)
{
    const char* cursor{start};
    int64_t exponent{};
    uint64_t significand{};
    int64_t exponentBias{};
    bool significandDotSeen{};
    double ret{};
    bool isOutOfRange{};
    bool precisionLost{};

    suffix = start;

    while (cursor != end)
    {
        uint64_t digit{};
        char c = *cursor;

        if ((c >= '0') && (c <= '9'))
            digit = static_cast<uint64_t>(c - '0');
        else if ((c >= 'A') && (c <= 'F'))
            digit = 10U + static_cast<uint64_t>(c - 'A');
        else if ((c >= 'a') && (c <= 'f'))
            digit = 10U + static_cast<uint64_t>(c - 'a');
        else if (!significandDotSeen && (c == '.'))
        {
            significandDotSeen = true;
            ++cursor;
            continue;
        }
        else
            break;

        // check whether the significand has room for this digit
        if ((significand & 0xF000000000000000U) == 0U)
        {
            significand <<= 4U;
            significand |= digit;

            if (significandDotSeen)
                exponentBias -= 4;
        }
        else
        {
            // No more room, so just update the exponent if we're on the left
            // side of the dot.
            if (!significandDotSeen)
                exponentBias += 4;

            // significand is being truncated
            if (digit != 0U)
                precisionLost = true;
        }

        ++cursor;
    }

    if (cursor != start)
        suffix = cursor;

    // do/while for breakable exit
    do
    {
        // significand parsed?
        if (cursor == start)
            break;

        // read 'p'/'P'
        if ((cursor == end) || ((*cursor != 'p') && (*cursor != 'P')))
            break;
        ++cursor;

        bool sign{};

        // read optional sign
        if ((cursor != end) && ((*cursor == '+') || (*cursor == '-')))
        {
            sign = (*cursor == '-');
            ++cursor;
        }

        while (cursor != end)
        {
            int64_t digit{};
            char c = *cursor;
            if ((c >= '0') && (c <= '9'))
                digit = static_cast<int64_t>(c - '0');
            else
                break;

            exponent *= 10;
            exponent += digit;
            exponent = std::min(exponent, std::numeric_limits<int64_t>::max() / 100);

            ++cursor;
            suffix = cursor;
        }

        if (sign)
            exponent = -exponent;

    } while (false);

    // was something parsed?
    if (suffix != start)
    {
        // start by applying exponent bias
        exponent += exponentBias;

        if (significand != 0U)
        {
            // normalize significand
            int leadingZeroes = std::countl_zero(significand);
            significand <<= leadingZeroes;
            exponent -= leadingZeroes;

            // truncate significand to 53 bits or less in case of subnormals
            if (significand & 0x7FFU)
                precisionLost = true;

            significand >>= 11U;
            exponent += 11;

            // Note: normal exponent range is [-1022, 1023]. Numbers smaller
            // than 2^-1022 are expressed as subnormals. In case of smaller
            // exponents, we'll clamp the final exponent range to the normal
            // range and shift the significand right. This prevents potential
            // issues with rounding.
            if (exponent < (-1022 - 52))
            {
                int64_t diff = (-1022 - 52) - exponent;
                if (diff > 52)
                {
                    significand = 0;
                    precisionLost = true;
                }
                else
                {
                    uint64_t lostBitsMask = (uint64_t{1U} << diff) - 1U;
                    if (significand & lostBitsMask)
                        precisionLost = true;
                    significand >>= diff;
                }

                exponent = (-1022 - 52);
            }

            // now calculate the actual value
            ret = static_cast<double>(significand);
            ret *= std::exp2(-52);
            exponent += 52;
            ret *= std::exp2(exponent); // apply exponent

            // detect underflow/overflow
            isOutOfRange = (ret == 0.0 || (!std::isfinite(ret)));
        }
        else
        {
            // if significand is 0, then the value is 0 no matter the exponent
            ret = 0.0;
            isOutOfRange = false;
        }
    }

    outIsOutOfRange = isOutOfRange;
    outPrecisionLost = precisionLost && !isOutOfRange;
    return ret;
}

FloatingPointLiteralValue getFloatingPointLiteralValue(
    Token const& token,
    FloatingPointLiteralType& outLiteralType,
    bool& outIsOutOfRange,
    bool& outPrecisionLost,
    UnownedStringSlice& outErrorContent)
{
    const UnownedStringSlice content = token.getContent();
    const char* cursor = content.begin();
    const char* end = content.end();

    bool hexFloat{};
    const char* numberStart{};
    UnownedStringSlice errorContent{};

    // start by consuming the hex prefix if any
    if (UnownedStringSlice(cursor, end).startsWith("0x") ||
        UnownedStringSlice(cursor, end).startsWith("0X"))
    {
        // Manual implementation for hex-to-double
        // translation. std::from_chars() does not work reliably on Mac and
        // fast_float only supports decimal floats. Fortunately, hex-to-double
        // is reasonably straightforward.

        cursor += 2U;
        hexFloat = true;
    }

    // the number starts here
    numberStart = cursor;

    // scan through the number
    if (hexFloat)
    {
        // Hex float: find exponent marker (p/P)
        while (cursor != end)
        {
            char c = *cursor;
            if (c == '#')
                break;
            ++cursor;
            if (c == 'p' || c == 'P')
                break;
        }

        // then scan through the exponent number
        while (cursor != end)
        {
            bool expChar{};
            switch (*cursor)
            {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-':
                expChar = true;
                break;

            default:
                expChar = false;
                break;
            }

            if (!expChar)
                break;

            ++cursor;
        }

        // rest is suffix.
    }
    else
    {
        // regular float: the number chars (incl. exponent) are distinct from
        // suffix chars
        while (cursor != end)
        {
            bool numberChar{};
            switch (*cursor)
            {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'e':
            case 'E':
            case '+':
            case '-':
            case '.':
                numberChar = true;
                break;

            default:
                numberChar = false;
                break;
            }

            if (!numberChar)
                break;

            ++cursor;
        }

        // rest is suffix.
    }

    UnownedStringSlice number{numberStart, cursor};
    FloatingPointLiteralValue value{};
    bool isInfinity{};

    // Check for infinity marker
    if (UnownedStringSlice(cursor, end).startsWith("#INF"))
    {
        isInfinity = true;
        cursor += 4U;
        value = std::numeric_limits<double>::infinity();
    }

    UnownedStringSlice suffix{cursor, end};

    // start from the suffix, since we need the literal type for rounding
    FloatingPointLiteralType literalType;
    if ((suffix == "") || (suffix == "f") || (suffix == "F"))
        literalType = FloatingPointLiteralType::Float;
    else if (
        (suffix == "h") || (suffix == "H") || (suffix == "hf") || (suffix == "HF") ||
        (suffix == "fh") || (suffix == "FH"))
        literalType = FloatingPointLiteralType::Half;
    else if (
        (suffix == "l") || (suffix == "L") || (suffix == "lf") || (suffix == "LF") ||
        (suffix == "fl") || (suffix == "FL"))
        literalType = FloatingPointLiteralType::Double;
    else
    {
        literalType = FloatingPointLiteralType::BadSuffix;
        errorContent = suffix;
    }

    // then the floating-point number
    bool isOutOfRange{};
    bool precisionLost{};

    // Cursor is updated to be at the end of parsed number
    if (isInfinity)
    {
        cursor = number.end();
    }
    else if (hexFloat)
    {
        value = _hexFloatLiteralToDouble(
            number.begin(),
            number.end(),
            cursor,
            isOutOfRange,
            precisionLost);

        double oldValue = value;

        if (literalType == FloatingPointLiteralType::Half)
            value = _truncateDouble(value, -14, +15, 11, false);
        else if (literalType == FloatingPointLiteralType::Float)
            value = _truncateDouble(value, -126, +127, 24, false);

        // value became 0 in truncation?
        if (oldValue != 0.0 && value == 0.0)
            isOutOfRange = true;

        // value changed in truncation?
        if (value != oldValue)
            precisionLost = true;
    }
    else
    {
        value = 0.0; // default in case of errors. fast_float sets the value
                     // appropriately in case of result_out_of_range

        fast_float::from_chars_result_t<char> result{};

        if (literalType == FloatingPointLiteralType::Float)
        {
            float f{};
            result = fast_float::from_chars(number.begin(), number.end(), f);
            value = f;
        }
        else if (literalType == FloatingPointLiteralType::Half)
        {
            // We do not currently have std::float16_t support in all our
            // compiler toolchains, so parse to double and then round to
            // half. This effectively performs double rounding (decimal ->
            // double -> half), and therefore in rare cases, the result may
            // differ from a single correctly-rounded decimal -> half.
            result = fast_float::from_chars(number.begin(), number.end(), value);

            if (result.ec == std::errc{})
            {
                double oldValue = value;
                value = _truncateDouble(value, -14, +15, 11, true);

                if (!std::isfinite(value))
                    isOutOfRange = true;
                else if (oldValue != 0.0 && value == 0.0)
                    isOutOfRange = true;
            }
        }
        else
        {
            // in all other cases, parse as double
            result = fast_float::from_chars(number.begin(), number.end(), value);
        }

        cursor = result.ptr;

        if (result.ec == std::errc::result_out_of_range)
        {
            // overflow-to-infinity or underflow-to-zero
            isOutOfRange = true;
        }
        else if (result.ec != std::errc{})
        {
            // nonspecific error
            literalType = FloatingPointLiteralType::BadSignificand;
            errorContent = UnownedStringSlice(content.begin(), number.end());
            value = 0.0;
        }
    }

    // check for special exponent for infinity
    if (cursor != number.end())
    {
        literalType = FloatingPointLiteralType::BadSignificand;
        errorContent = UnownedStringSlice(content.begin(), number.end());
    }

    // report results
    outLiteralType = literalType;
    outIsOutOfRange = isOutOfRange;
    outPrecisionLost = precisionLost && !isOutOfRange;
    outErrorContent = errorContent;

    return value;
}

static bool _isHexDigit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static uint32_t _parseHexNumber(
    const char*& cursor,
    const char* const e,
    size_t maxDigits,
    size_t& outNumDigits,
    bool& outOverflow)
{
    uint32_t value{};
    size_t numDigits{};
    bool overflow{};

    while ((cursor != e) && (numDigits < maxDigits) && _isHexDigit(*cursor))
    {
        uint8_t digit{};
        if (*cursor >= '0' && *cursor <= '9')
            digit = *cursor - '0';
        else if (*cursor >= 'A' && *cursor <= 'F')
            digit = 10 + (*cursor - 'A');
        else
            digit = 10 + (*cursor - 'a');

        if (value & uint64_t{0xF000'0000})
            overflow = true;

        value <<= 4U;
        value |= digit;

        ++numDigits;
        ++cursor;
    }

    outOverflow = overflow;
    outNumDigits = numDigits;
    return value;
}

// Decodes string escape sequence, returns -1 on failure. Failures will be
// diagnosed.
//
// Preconditions:
// - cursor != e      -- there must be at least 1 character
// - *cursor == '\\'  -- first char of escape sequence must be backslash
static IntegerLiteralValue _decodeStringEscape(
    const char*& cursor,
    const char* const e,
    DiagnosticSink* sink,
    const SourceLoc& loc,
    bool& outUnicode)
{
    const char* const b = cursor;
    bool unicode{};
    int64_t value{};

    // this should have been already checked before we get here
    SLANG_ASSERT(*cursor == '\\');
    SLANG_ASSERT(cursor != e);

    ++cursor;

    // check that there is an escape sequence after '\'
    if (cursor == e)
    {
        diagnose(sink, loc, LexerDiagnostics::invalidStringEscape, String(b, cursor));
        value = -1;
    }
    else
    {
        uint8_t byte = *cursor++;
        switch (byte)
        {
        case '\'':
            value = '\'';
            break;

        case '"':
            value = '"';
            break;

        case '\\':
            value = '\\';
            break;

        case '?':
            value = '?';
            break;

        case 'a':
            value = 7;
            break;

        case 'b':
            value = 8;
            break;

        case 'f':
            value = 12;
            break;

        case 'n':
            value = 10;
            break;

        case 'r':
            value = 13;
            break;

        case 't':
            value = 9;
            break;

        case 'v':
            value = 11;
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            {
                value = byte - '0';
                size_t numChars = 1U;
                while (cursor != e && *cursor >= '0' && *cursor <= '7' && numChars < 3U)
                {
                    value <<= 3;
                    value |= static_cast<uint8_t>(*cursor - '0');
                    ++cursor;
                    ++numChars;
                }
                break;
            }

        case 'x':
            {
                bool overflow{};
                size_t numDigits{};

                if ((cursor != e) && (*cursor == '{'))
                {
                    // \x{...}

                    ++cursor;
                    value = _parseHexNumber(cursor, e, 255U, numDigits, overflow);

                    if ((cursor != e) && (*cursor == '}'))
                        ++cursor;
                    else
                    {
                        // no enclosing '}'
                        value = -1;
                    }
                }
                else
                {
                    // \x...
                    value = _parseHexNumber(cursor, e, 255U, numDigits, overflow);
                }

                if ((numDigits == 0U) || overflow)
                    value = -1;

                if (value == -1)
                    diagnose(sink, loc, LexerDiagnostics::invalidStringEscape, String(b, cursor));

                break;
            }

        case 'u':
            {
                bool overflow{};
                size_t numDigits{};

                if ((cursor != e) && (*cursor == '{'))
                {
                    // \u{...}

                    ++cursor;
                    value = _parseHexNumber(cursor, e, 255U, numDigits, overflow);

                    if ((cursor != e) && (*cursor == '}'))
                    {
                        ++cursor;

                        if (numDigits == 0U)
                            value = -1;
                    }
                    else
                    {
                        // no enclosing '}'
                        value = -1;
                    }

                    if (overflow)
                        value = -1;

                    if (value == -1)
                        diagnose(
                            sink,
                            loc,
                            LexerDiagnostics::invalidStringEscape,
                            String(b, cursor));
                }
                else
                {
                    // \u...
                    value = _parseHexNumber(cursor, e, 4U, numDigits, overflow);

                    // note: value cannot overflow here (4 hex digits max), so we'll
                    // guard this with assert instead of check to avoid unreachable
                    // branches
                    SLANG_ASSERT(!overflow);

                    if (numDigits != 4U)
                    {
                        diagnose(sink, loc, LexerDiagnostics::invalidUnicodeStringEscape, "u", 4U);
                        value = -1;
                    }
                }

                unicode = true;
                break;
            }

        case 'U':
            {
                bool overflow{};
                size_t numDigits{};

                value = _parseHexNumber(cursor, e, 8U, numDigits, overflow);

                // note: value cannot overflow here (8 hex digits max), so we'll
                // guard this with assert instead of check to avoid unreachable
                // branches
                SLANG_ASSERT(!overflow);

                if (numDigits != 8U)
                {
                    diagnose(sink, loc, LexerDiagnostics::invalidUnicodeStringEscape, "U", 8U);
                    value = -1;
                }

                unicode = true;

                break;
            }

        default:
            {
                diagnose(sink, loc, LexerDiagnostics::invalidStringEscape, String(b, cursor));
                value = -1;
                break;
            }
        }
    }

    outUnicode = unicode;
    return value;
}

IntegerLiteralValue getCharLiteralValue(Token const& token, DiagnosticSink* sink)
{
    // See docs/language-reference/expressions-literal.md for the literal format.

    UnownedStringSlice content = token.getContent();

    // sanity check
    if (content.getLength() < 3U)
    {
        diagnose(sink, token.getLoc(), LexerDiagnostics::illegalCharacterLiteral);
        return -1;
    }

    // unquoted begin/end pointers
    const char* const b = content.begin() + 1;
    const char* const e = content.end() - 1;
    const char* cursor = b;
    IntegerLiteralValue ret{};

    if (*cursor == '\\')
    {
        // handle escape
        bool unicode{};
        ret = _decodeStringEscape(cursor, e, sink, token.getLoc(), unicode);

        // note: unicode matters only with string literals
        static_cast<void>(unicode);
    }
    else
    {
        bool invalid{};
        ret = getUnicodePointFromUTF8(
            [&cursor, e]() -> unsigned char
            {
                if (cursor < e)
                    return static_cast<unsigned char>(*cursor++);
                else
                {
                    // 0xFF is invalid byte anywhere in UTF-8, so this will
                    // trigger an error in the decoder
                    return 0xFFU;
                }
            },
            &invalid);

        if (invalid)
        {
            diagnose(
                sink,
                token.getLoc(),
                LexerDiagnostics::invalidUtf8ByteSequence,
                _inputToByteSeq(b, cursor).getBuffer());

            ret = -1;
        }
    }

    // did we consume everything? (and we haven't diagnosed yet)
    if ((cursor != e) && (ret != -1))
    {
        diagnose(sink, token.getLoc(), LexerDiagnostics::illegalCharacterLiteral);
        ret = -1;
    }

    return ret;
}

// We use string literals in places where we don't do string escaping other than
// the bare minimum (\"). Therefore, this function pushes almost all
// escape-related checking and error handling to getStringLiteralTokenValue()
// and getCharLiteralValue().
static void _lexStringLiteralBody(Lexer* lexer, char quote)
{
    for (;;)
    {
        int c = _peek(lexer);
        if (c == quote)
        {
            _advance(lexer);
            return;
        }

        switch (c)
        {
        case kEOF:
            if (auto sink = lexer->getDiagnosticSink())
            {
                diagnose(sink, _getSourceLoc(lexer), LexerDiagnostics::endOfFileInLiteral);
            }
            return;

        case '\n':
        case '\r':
            if (auto sink = lexer->getDiagnosticSink())
            {
                diagnose(sink, _getSourceLoc(lexer), LexerDiagnostics::newlineInLiteral);
            }
            return;

        case '\\':
            // We'll do only the bare minimum escape processing to detect
            // correctly the end of the literal. The escape diagnostics is done
            // in _decodeStringEscape() invoked by getStringLiteralTokenValue()
            // and getCharLiteralValue().
            _advance(lexer);
            switch (_peek(lexer))
            {
            case '\'':
            case '\"':
            case '\\':
                _advance(lexer);
                break;
            }
            break;

        default:
            _advance(lexer);
            continue;
        }
    }
}

static void _lexRawStringLiteralBody(Lexer* lexer)
{
    const char* start = lexer->m_cursor;
    const char* endOfDelimiter = nullptr;
    for (;;)
    {
        int c = _peek(lexer);
        if (c == '(' && endOfDelimiter == nullptr)
            endOfDelimiter = lexer->m_cursor;
        if (c == '\"')
        {
            if (!endOfDelimiter)
            {
                if (auto sink = lexer->getDiagnosticSink())
                {
                    diagnose(sink, _getSourceLoc(lexer), LexerDiagnostics::quoteCannotBeDelimiter);
                }
            }
            else
            {
                auto testStart = lexer->m_cursor - (endOfDelimiter - start);
                if (testStart > endOfDelimiter)
                {
                    auto testDelimiter = UnownedStringSlice(testStart, lexer->m_cursor);
                    auto delimiter = UnownedStringSlice(start, endOfDelimiter);
                    if (*(testStart - 1) == ')' && testDelimiter == delimiter)
                    {
                        _advance(lexer);
                        return;
                    }
                }
            }
        }

        switch (c)
        {
        case kEOF:
            if (auto sink = lexer->getDiagnosticSink())
            {
                diagnose(sink, _getSourceLoc(lexer), LexerDiagnostics::endOfFileInLiteral);
            }
            return;
        default:
            _advance(lexer);
            continue;
        }
    }
}

UnownedStringSlice getRawStringLiteralTokenValue(Token const& token)
{
    auto content = token.getContent();
    if (content.getLength() <= 5)
        return UnownedStringSlice();
    auto start = content.begin() + 2;
    auto delimEnd = start;
    while (delimEnd < content.end() && *delimEnd != '(')
        delimEnd++;
    auto delimLength = delimEnd - start;
    auto contentEnd = content.end() - delimLength - 2;
    auto contentBegin = start + delimLength + 1;
    if (contentEnd <= contentBegin)
        return UnownedStringSlice();
    return UnownedStringSlice(contentBegin, contentEnd);
}

String getStringLiteralTokenValue(Token const& token, DiagnosticSink* sink)
{
    SLANG_ASSERT(token.type == TokenType::StringLiteral || token.type == TokenType::CharLiteral);

    if (token.getContent().startsWith("R"))
        return getRawStringLiteralTokenValue(token);

    const UnownedStringSlice content = token.getContent();

    char const* cursor = content.begin();
    char const* end = content.end();

    // The token's content slice is a precise view of the lexed source text.
    // For an unterminated literal (newline / EOF inside a string), the lexer
    // emits a diagnostic but still produces a token whose content slice ends
    // before any closing quote. Every cursor advance below must therefore be
    // gated on `cursor < end` to avoid an out-of-bounds read into the
    // surrounding heap (#11278).
    if (cursor == end)
        return String();

    auto quote = *cursor++;
    SLANG_ASSERT(quote == '\'' || quote == '"');

    StringBuilder valueBuilder;
    while (cursor < end)
    {
        auto c = *cursor;

        // If we see a closing quote, then we are at the end of the string literal
        if (c == quote)
        {
            ++cursor;
            SLANG_ASSERT(cursor == end);
            return valueBuilder.produceString();
        }

        // Characters that don't being escape sequences are easy;
        // just append them to the buffer and move on.
        if (c != '\\')
        {
            ++cursor;
            valueBuilder.append(c);
            continue;
        }

        // decode escape sequence
        bool unicode{};
        IntegerLiteralValue charValue =
            _decodeStringEscape(cursor, end, sink, token.getLoc(), unicode);

        if (charValue >= 0)
        {
            if (!unicode)
            {
                // single byte
                if (charValue <= 255)
                {
                    valueBuilder.append(static_cast<char>(charValue));
                }
                else
                {
                    String hexSB;
                    hexSB.append(charValue, 16);
                    diagnose(
                        sink,
                        token.getLoc(),
                        LexerDiagnostics::outOfRangeCodeUnit,
                        charValue,
                        hexSB.getBuffer());
                }
            }
            else
            {
                char buffer[4]{};
                size_t numBytes = encodeUnicodePointToUTF8(static_cast<Char32>(charValue), buffer);
                SLANG_ASSERT(numBytes <= 4);

                if (numBytes >= 1)
                {
                    valueBuilder.append(buffer, numBytes);
                }
                else
                {
                    String hexSB;
                    hexSB.append(charValue, 16);
                    diagnose(
                        sink,
                        token.getLoc(),
                        LexerDiagnostics::outOfRangeCodePointForUtf8,
                        charValue,
                        hexSB.getBuffer());
                }
            }
        }
    }

    // Unterminated literal: return what we have built so far rather than
    // continuing to read past `end`. The lexer has already emitted a
    // diagnostic for the unterminated literal at this point.
    return valueBuilder.produceString();
}

String getFileNameTokenValue(Token const& token)
{
    const UnownedStringSlice content = token.getContent();

    // A file name usually doesn't process escape sequences
    // (this is import on Windows, where `\\` is a valid
    // path separator character).

    // Just trim off the first and last characters to remove the quotes
    // (whether they were `""` or `<>`.
    if (content.getLength() < 2)
        return String();
    return String(content.begin() + 1, content.end() - 1);
}

static TokenType _lexTokenImpl(Lexer* lexer)
{
    int nextCodePoint = _peek(lexer);
    switch (nextCodePoint)
    {
    default:
        break;

    case kEOF:
        return TokenType::EndOfFile;

    case '\r':
    case '\n':
        _handleNewLine(lexer);
        return TokenType::NewLine;

    case ' ':
    case '\t':
        _lexHorizontalSpace(lexer);
        return TokenType::WhiteSpace;

    case '.':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return _lexNumberAfterDecimalPoint(lexer, 10);

        case '.':
            // Note: consuming the second `.` here means that
            // we cannot back up and return a `.` token by itself
            // any more. We thus end up having distinct tokens for
            // `.`, `..`, and `...` even though the `..` case is
            // not part of HLSL.
            //
            _advance(lexer);
            switch (_peek(lexer))
            {
            case '.':
                _advance(lexer);
                return TokenType::Ellipsis;

            default:
                return TokenType::DotDot;
            }

        default:
            return TokenType::Dot;
        }

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return _lexNumber(lexer, 10);

    case '0':
        {
            auto loc = _getSourceLoc(lexer);
            _advance(lexer);
            switch (_peek(lexer))
            {
            default:
                // A bare `0` may be followed by either of the two
                // floating-point continuations recognised elsewhere by the
                // lexer:
                //   * the legacy MSVC `#INF` infinity form (so `0#INF`
                //     matches `1#INF`, which already worked through the
                //     1-9 → `_lexNumber` → `_maybeLexNumberExponent` path;
                //     #11276 Case 2),
                //   * a base-10 exponent `e`/`E` (so `0e10`, `0E5`,
                //     `0e+1`, `0e-3` lex as `FloatingPointLiteral`,
                //     matching `1e10` etc.). Without this the bare-`0`
                //     `default:` arm dropped through `_maybeLexNumberSuffix`
                //     and the alphanumeric-suffix loop swallowed `e10` as a
                //     literal suffix, producing an `IntegerLiteral` with
                //     text `0e10`.
                // For any other trailing character the `0` is just an
                // integer literal.
                if (_maybeLexNumberExponent(lexer, 10))
                    return _maybeLexNumberSuffix(lexer, TokenType::FloatingPointLiteral);
                return _maybeLexNumberSuffix(lexer, TokenType::IntegerLiteral);

            case '.':
                switch (_peek(lexer, 1))
                {
                    // 0.xxxx or 0.rrrr
                case 'x':
                case 'r':
                    return _maybeLexNumberSuffix(lexer, TokenType::IntegerLiteral);
                default:
                    _advance(lexer);
                    return _lexNumberAfterDecimalPoint(lexer, 10);
                }

            case 'x':
            case 'X':
                _advance(lexer);
                return _lexNumber(lexer, 16);

            case 'b':
            case 'B':
                _advance(lexer);
                return _lexNumber(lexer, 2);

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (auto sink = lexer->getDiagnosticSink())
                {
                    diagnose(sink, loc, LexerDiagnostics::octalLiteral);
                }
                return _lexNumber(lexer, 8);
            }
        }

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    case '_':
        _lexIdentifier(lexer);
        return TokenType::Identifier;
    case 'R':
        _advance(lexer);
        switch (_peek(lexer))
        {
        default:
            _lexIdentifier(lexer);
            return TokenType::Identifier;
        case '\"':
            _advance(lexer);
            _lexRawStringLiteralBody(lexer);
            return TokenType::StringLiteral;
        }

    case '\"':
        _advance(lexer);
        _lexStringLiteralBody(lexer, '\"');
        return TokenType::StringLiteral;

    case '\'':
        _advance(lexer);
        _lexStringLiteralBody(lexer, '\'');
        return TokenType::CharLiteral;


    case '+':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '+':
            _advance(lexer);
            return TokenType::OpInc;
        case '=':
            _advance(lexer);
            return TokenType::OpAddAssign;
        default:
            return TokenType::OpAdd;
        }

    case '-':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '-':
            _advance(lexer);
            return TokenType::OpDec;
        case '=':
            _advance(lexer);
            return TokenType::OpSubAssign;
        case '>':
            _advance(lexer);
            return TokenType::RightArrow;
        default:
            return TokenType::OpSub;
        }

    case '*':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '=':
            _advance(lexer);
            return TokenType::OpMulAssign;
        default:
            return TokenType::OpMul;
        }

    case '/':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '=':
            _advance(lexer);
            return TokenType::OpDivAssign;
        case '/':
            _advance(lexer);
            _lexLineComment(lexer);
            return TokenType::LineComment;
        case '*':
            _advance(lexer);
            _lexBlockComment(lexer);
            return TokenType::BlockComment;
        default:
            return TokenType::OpDiv;
        }

    case '%':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '=':
            _advance(lexer);
            return TokenType::OpModAssign;
        default:
            return TokenType::OpMod;
        }

    case '|':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '|':
            _advance(lexer);
            return TokenType::OpOr;
        case '=':
            _advance(lexer);
            return TokenType::OpOrAssign;
        default:
            return TokenType::OpBitOr;
        }

    case '&':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '&':
            _advance(lexer);
            return TokenType::OpAnd;
        case '=':
            _advance(lexer);
            return TokenType::OpAndAssign;
        default:
            return TokenType::OpBitAnd;
        }

    case '^':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '=':
            _advance(lexer);
            return TokenType::OpXorAssign;
        default:
            return TokenType::OpBitXor;
        }

    case '>':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '>':
            _advance(lexer);
            switch (_peek(lexer))
            {
            case '=':
                _advance(lexer);
                return TokenType::OpShrAssign;
            default:
                return TokenType::OpRsh;
            }
        case '=':
            _advance(lexer);
            return TokenType::OpGeq;
        default:
            return TokenType::OpGreater;
        }

    case '<':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '<':
            _advance(lexer);
            switch (_peek(lexer))
            {
            case '=':
                _advance(lexer);
                return TokenType::OpShlAssign;
            default:
                return TokenType::OpLsh;
            }
        case '=':
            _advance(lexer);
            return TokenType::OpLeq;
        default:
            return TokenType::OpLess;
        }

    case '=':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '=':
            _advance(lexer);
            return TokenType::OpEql;
        case '>':
            _advance(lexer);
            return TokenType::DoubleRightArrow;
        default:
            return TokenType::OpAssign;
        }

    case '!':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '=':
            _advance(lexer);
            return TokenType::OpNeq;
        default:
            return TokenType::OpNot;
        }

    case '#':
        _advance(lexer);
        switch (_peek(lexer))
        {
        case '#':
            _advance(lexer);
            return TokenType::PoundPound;

        case '?':
            _advance(lexer);
            return TokenType::CompletionRequest;

        default:
            return TokenType::Pound;
        }

    case '~':
        _advance(lexer);
        return TokenType::OpBitNot;

    case ':':
        {
            _advance(lexer);
            if (_peek(lexer) == ':')
            {
                _advance(lexer);
                return TokenType::Scope;
            }
            return TokenType::Colon;
        }
    case ';':
        _advance(lexer);
        return TokenType::Semicolon;
    case ',':
        _advance(lexer);
        return TokenType::Comma;

    case '{':
        _advance(lexer);
        return TokenType::LBrace;
    case '}':
        _advance(lexer);
        return TokenType::RBrace;
    case '[':
        _advance(lexer);
        return TokenType::LBracket;
    case ']':
        _advance(lexer);
        return TokenType::RBracket;
    case '(':
        _advance(lexer);
        return TokenType::LParent;
    case ')':
        _advance(lexer);
        return TokenType::RParent;

    case '?':
        _advance(lexer);
        return TokenType::QuestionMark;
    case '@':
        _advance(lexer);
        return TokenType::At;
    case '$':
        {
            _advance(lexer);
            if (_peek(lexer) == '$')
            {
                _advance(lexer);
                return TokenType::DollarDollar;
            }
            return TokenType::Dollar;
        }
    }

    // We treat all unicode characters as a part of an identifier.
    if (isNonAsciiCodePoint(nextCodePoint))
    {
        _lexIdentifier(lexer);
        return TokenType::Identifier;
    }

    {
        // If none of the above cases matched, then we have an
        // unexpected/invalid character.

        auto loc = _getSourceLoc(lexer);
        int c = _advance(lexer);

        if (auto sink = lexer->getDiagnosticSink())
        {
            if (c >= 0x20 && c <= 0x7E)
            {
                char buffer[] = {(char)c, 0};
                diagnose(sink, loc, LexerDiagnostics::illegalCharacterPrint, buffer);
            }
            else if (c == kEOF)
            {
                diagnose(sink, loc, LexerDiagnostics::unexpectedEndOfInput);
            }
            else
            {
                // Fallback: print as hexadecimal
                diagnose(
                    sink,
                    loc,
                    LexerDiagnostics::illegalCharacterHex,
                    String((unsigned char)c, 16));
            }
        }

        return TokenType::Invalid;
    }
}

Token Lexer::lexToken()
{
    for (;;)
    {
        Token token;
        token.loc = _getSourceLoc(this);

        char const* textBegin = m_cursor;

        auto tokenType = _lexTokenImpl(this);

        // The flags on the token we just lexed will be based
        // on the current state of the lexer.
        //
        auto tokenFlags = m_tokenFlags;
        //
        // Depending on what kind of token we just lexed, the
        // flags that will be used for the *next* token might
        // need to be updated.
        //
        switch (tokenType)
        {
        case TokenType::NewLine:
            {
                // If we just reached the end of a line, then the next token
                // should count as being at the start of a line, and also after
                // whitespace.
                //
                m_tokenFlags = TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace;
                break;
            }

        case TokenType::WhiteSpace:
        case TokenType::BlockComment:
        case TokenType::LineComment:
            {
                // True horizontal whitespace and comments both count as whitespace.
                //
                // Note that a line comment does not include the terminating newline,
                // we do not need to set `AtStartOfLine` here.
                //
                m_tokenFlags |= TokenFlag::AfterWhitespace;
                break;
            }

        default:
            {
                // If we read some token other then the above cases, then we are
                // neither after whitespace nor at the start of a line.
                //
                m_tokenFlags = 0;
                break;
            }
        }

        token.type = tokenType;
        token.flags = tokenFlags;

        char const* textEnd = m_cursor;

        // Note(tfoley): `StringBuilder::Append()` seems to crash when appending zero bytes
        if (textEnd != textBegin)
        {
            // "scrubbing" token value here to remove escaped newlines...
            //
            // Only perform this work if we encountered an escaped newline
            // while lexing this token (e.g., keep a flag on the lexer), or
            // do it on-demand when the actual value of the token is needed.
            if (tokenFlags & TokenFlag::ScrubbingNeeded)
            {
                // Allocate space that will always be more than enough for stripped contents
                char* startDst = (char*)m_memoryArena->allocateUnaligned(textEnd - textBegin);
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
                        case '\r':
                        case '\n':
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
                token.setContent(UnownedStringSlice(startDst, dst));
            }
            else
            {
                token.setContent(UnownedStringSlice(textBegin, textEnd));
            }
        }

        if (m_namePool)
        {
            if (tokenType == TokenType::Identifier || tokenType == TokenType::CompletionRequest)
            {
                token.setName(m_namePool->getName(token.getContent()));
            }
        }

        return token;
    }
}

TokenList Lexer::lexAllSemanticTokens()
{
    TokenList tokenList;
    for (;;)
    {
        Token token = lexToken();

        // We are only interested intokens that are semantically
        // significant, so we will skip over forms of whitespace
        // and comments.
        //
        switch (token.type)
        {
        default:
            break;

        case TokenType::WhiteSpace:
        case TokenType::BlockComment:
        case TokenType::LineComment:
        case TokenType::NewLine:
            continue;
        }

        tokenList.add(token);
        if (token.type == TokenType::EndOfFile)
            return tokenList;
    }
}

TokenList Lexer::lexAllMarkupTokens()
{
    TokenList tokenList;
    for (;;)
    {
        Token token = lexToken();
        switch (token.type)
        {
        default:
            break;

        case TokenType::WhiteSpace:
        case TokenType::NewLine:
            continue;
        }

        tokenList.add(token);
        if (token.type == TokenType::EndOfFile)
            return tokenList;
    }
}

TokenList Lexer::lexAllTokens()
{
    TokenList tokenList;
    for (;;)
    {
        Token token = lexToken();
        tokenList.add(token);
        if (token.type == TokenType::EndOfFile)
            return tokenList;
    }
}

/* static */ UnownedStringSlice Lexer::sourceLocationLexer(const UnownedStringSlice& in)
{
    Lexer lexer;

    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);

    auto sourceFile = sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), in);
    auto sourceView = sourceManager.createSourceView(sourceFile, nullptr, SourceLoc::fromRaw(0));

    DiagnosticSink sink(&sourceManager, nullptr);

    MemoryArena arena;

    NamePool namePool;

    lexer.initialize(sourceView, &sink, &namePool, &arena);

    Token tok = lexer.lexToken();

    if (tok.type == TokenType::Invalid)
    {
        return UnownedStringSlice();
    }

    const int offset = sourceView->getRange().getOffset(tok.loc);

    if (offset < 0 || offset >= in.getLength())
        return UnownedStringSlice();
    if (Index(offset + tok.charsCount) > in.getLength())
        return UnownedStringSlice();

    return UnownedStringSlice(in.begin() + offset, in.begin() + offset + tok.charsCount);
}

SourceLoc Lexer::findNextLineEnd(SourceLoc from, UInt& lineCount) const
{
    const char* it = m_begin + (from.getRaw() - m_startLoc.getRaw());
    if (it >= m_begin && it < m_end)
    {
        while (it != m_end)
        {
            const char c = *it;
            if (c == '\n' || c == '\r')
            {
                const char next = ((it + 1) == m_end) ? char(kEOF) : *(it + 1);
                if ((next ^ c) == ('\n' ^ '\r'))
                {
                    ++it;
                }
                --lineCount;
                if (lineCount == 0)
                {
                    SourceLoc res = _getSourceLoc(*this, it);
                    return res;
                }
            }
            ++it;
        }
    }
    return {};
}

} // namespace Slang
