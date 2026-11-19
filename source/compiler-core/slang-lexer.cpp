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
Token TokenReader::readToken()
{
    Token result = peekToken();
    if (m_cursor != m_end)
        ++m_cursor;
    _updateLookaheadToken();
    return result;
}

// Lexer

void Lexer::initialize(
    SourceView* sourceView,
    DiagnosticSink* sink,
    NamePool* namePool,
    TokenPool* tokenPool)
{
    m_sourceView = sourceView;
    m_sink = sink;
    m_namePool = namePool;
    SLANG_RELEASE_ASSERT(tokenPool);
    m_tokenPool = tokenPool;
    m_mode = LexerMode::Normal;
    // Reinitialization can select a different pool. We discard its borrowed
    // pointers while retaining only the capacity of the lexer's own buffers.
    m_leadingTrivia.clear();
    m_trailingTrivia.clear();

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

// The byte cursor separates bounded input access from newline splicing and UTF-8
// decoding. Peeking can copy this cursor without changing the lexer's state.
struct RawByteCursor
{
    const char* position;
    const char* end;
};

// Raw reads return an unsigned byte or EOF, including for a non-terminated buffer.
static int _peekRawByte(const RawByteCursor& cursor, Index offset = 0)
{
    SLANG_RELEASE_ASSERT(offset >= 0);
    if (offset >= cursor.end - cursor.position)
        return kEOF;
    return (unsigned char)cursor.position[offset];
}

static int _readRawByte(RawByteCursor& cursor)
{
    int result = _peekRawByte(cursor);
    if (result != kEOF)
        ++cursor.position;
    return result;
}

static int _peekRawByte(const Lexer* lexer, Index offset = 0)
{
    return _peekRawByte(RawByteCursor{lexer->m_cursor, lexer->m_end}, offset);
}

static int _readRawByte(Lexer* lexer)
{
    RawByteCursor cursor{lexer->m_cursor, lexer->m_end};
    int result = _readRawByte(cursor);
    lexer->m_cursor = cursor.position;
    return result;
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

    int d = _peekRawByte(lexer);
    if ((c ^ d) == ('\n' ^ '\r'))
    {
        _readRawByte(lexer);
    }
}

// Reads a logical code point after removing backslash-newline pairs, using a space
// for failed UTF-8 decoding so identifier and number scanning stop at the failed sequence.
// Both lookahead and consumption use this procedure so they recognize identical input.
// Only consumption supplies a diagnostic sink and commits the resulting cursor.
static int _readCodePoint(
    const Lexer* lexer,
    RawByteCursor& cursor,
    bool& hasLineSplice,
    DiagnosticSink* sink)
{
    // A backslash is ordinary input unless a newline immediately follows it.
    // In particular, if it is the last byte, the raw peek returns EOF and we
    // return the backslash without trying to inspect memory past the buffer.
    int c;
    for (;;)
    {
        c = _readRawByte(cursor);
        if (c != '\\')
            break;
        int next = _peekRawByte(cursor);
        if (next != '\r' && next != '\n')
            break;

        // A splice removes one complete newline sequence, including CRLF or LFCR.
        // We then try again because several splices can precede one code point.
        _readRawByte(cursor);
        if ((next ^ _peekRawByte(cursor)) == ('\r' ^ '\n'))
            _readRawByte(cursor);
        hasLineSplice = true;
    }
    if (c == kEOF)
        return kEOF;

    // UTF-8 decoding may request several bytes. A missing or non-continuation
    // byte makes the sequence invalid; a non-continuation byte remains unread
    // so that the next code point can still be recognized normally.
    if (isUtf8LeadingByte((Byte)c))
    {
        --cursor.position;
        const char* sequenceStart = cursor.position;
        bool isInvalid = false;
        auto codePoint = getUnicodePointFromUTF8(
            [&]() -> unsigned char
            {
                int next = _peekRawByte(cursor);
                if (next == kEOF)
                    return 0;
                if (cursor.position == sequenceStart)
                    return (unsigned char)_readRawByte(cursor);
                if (isUtf8ContinuationByte((Byte)next))
                    return (unsigned char)_readRawByte(cursor);
                return 0xFF;
            },
            &isInvalid);
        // The shared decoder also accepts values outside Unicode. Lexical input
        // requires Unicode scalar values; rejecting the wider range before an
        // `int` conversion also keeps decoded input distinct from the EOF sentinel.
        if (codePoint > 0x10FFFF)
            isInvalid = true;
        if (codePoint >= 0xD800 && codePoint <= 0xDFFF)
            isInvalid = true;
        if (isInvalid)
        {
            // We have consumed the invalid bytes so the next read can make progress.
            // The lexer still retains those bytes in the lexeme's physical spelling.
            diagnose(
                sink,
                _getSourceLoc(*lexer, sequenceStart),
                LexerDiagnostics::invalidUtf8ByteSequence,
                _inputToByteSeq(sequenceStart, cursor.position).getBuffer());
            return ' ';
        }
        c = int(codePoint);
    }

    // A zero byte ends lexical input even when the containing view has more bytes.
    if (c == 0)
        cursor.position = cursor.end;
    return c;
}

/// Returns the code point `offset` logical positions ahead, or `kEOF` at the view's end.
///
/// Requires `offset >= 0`. Lookahead leaves the cursor and diagnostics unchanged.
static int _peekSlow(Lexer* lexer, int offset)
{
    RawByteCursor cursor{lexer->m_cursor, lexer->m_end};
    bool hasLineSplice = false;
    int result;
    do
    {
        result = _readCodePoint(lexer, cursor, hasLineSplice, nullptr);
    } while (offset--);
    return result;
}

/// Reads a logical code point and advances the byte cursor, or returns `kEOF` at the view's end.
///
/// Failed UTF-8 decoding yields a recovery space. Diagnostics obey lexer suppression and limits.
/// Removed line splices are recorded for the current lexeme's logical spelling.
static int _advanceSlow(Lexer* lexer)
{
    RawByteCursor cursor{lexer->m_cursor, lexer->m_end};
    bool hasLineSplice = false;
    int result = _readCodePoint(lexer, cursor, hasLineSplice, lexer->getDiagnosticSink());
    lexer->m_cursor = cursor.position;
    if (hasLineSplice)
        lexer->m_hasLineSpliceInCurrentLexeme = true;
    return result;
}

// We can return an ordinary ASCII byte without setting up the full decoder.
// NUL and EOF need its end-of-input handling, a backslash may begin a splice,
// and non-ASCII bytes need UTF-8 decoding or validation.
static SLANG_FORCE_INLINE bool _canReadAsciiCodePointDirectly(int rawByte)
{
    if (rawByte <= 0)
        return false;
    if (rawByte >= 0x80)
        return false;
    return rawByte != '\\';
}

// Most scanner decisions inspect the immediately following ASCII character.
// We handle that case directly and retain logical code-point lookahead for
// offsets or bytes that require the full decoder's input handling.
static SLANG_FORCE_INLINE int _peek(Lexer* lexer, int offset = 0)
{
    if (offset != 0)
        return _peekSlow(lexer, offset);
    const int rawByte = _peekRawByte(lexer);
    if (_canReadAsciiCodePointDirectly(rawByte))
        return rawByte;
    return _peekSlow(lexer, 0);
}

// Consuming an ordinary ASCII character only advances the byte cursor. We
// select a diagnostic sink and update splice state only on the full path,
// because an ordinary ASCII byte cannot require either operation.
static SLANG_FORCE_INLINE int _advance(Lexer* lexer)
{
    const int rawByte = _peekRawByte(lexer);
    if (_canReadAsciiCodePointDirectly(rawByte))
    {
        ++lexer->m_cursor;
        return rawByte;
    }
    return _advanceSlow(lexer);
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

    const UnownedStringSlice content = token.getLogicalSpelling();

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
    const UnownedStringSlice content = token.getLogicalSpelling();
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

IntegerLiteralValue getCharLiteralValue(RawCharLiteral token, DiagnosticSink* sink)
{
    SLANG_RELEASE_ASSERT(token);
    UnownedStringSlice content = token.getLogicalSpelling();

    // We require two quotes and at least one byte before decoding. The lexer can produce
    // an incomplete character literal after diagnosing a newline or EOF inside it.
    if (content.getLength() < 3U)
    {
        diagnose(sink, token.getLoc(), LexerDiagnostics::illegalCharacterLiteral);
        return -1;
    }
    SLANG_RELEASE_ASSERT(content[0] == '\'');
    if (content.end()[-1] != '\'')
    {
        diagnose(sink, token.getLoc(), LexerDiagnostics::illegalCharacterLiteral);
        return -1;
    }

    // We decode exactly one escaped character or UTF-8 code point from the quoted body.
    const char* const bodyBegin = content.begin() + 1;
    const char* const bodyEnd = content.end() - 1;
    const char* cursor = bodyBegin;
    IntegerLiteralValue value{};

    if (*cursor == '\\')
    {
        bool isUnicodeEscape{};
        value = _decodeStringEscape(cursor, bodyEnd, sink, token.getLoc(), isUnicodeEscape);

        // Both escape forms denote a code point in a character literal; only strings need
        // to distinguish a single-byte escape from a Unicode escape when producing bytes.
        static_cast<void>(isUnicodeEscape);
    }
    else
    {
        bool hasInvalidUtf8{};
        value = getUnicodePointFromUTF8(
            [&cursor, bodyEnd]() -> unsigned char
            {
                if (cursor < bodyEnd)
                    return static_cast<unsigned char>(*cursor++);
                else
                {
                    // We signal truncated UTF-8 to the decoder with an invalid UTF-8 byte
                    // instead of reading beyond the literal's body.
                    return 0xFFU;
                }
            },
            &hasInvalidUtf8);

        if (hasInvalidUtf8)
        {
            diagnose(
                sink,
                token.getLoc(),
                LexerDiagnostics::invalidUtf8ByteSequence,
                _inputToByteSeq(bodyBegin, cursor).getBuffer());

            value = -1;
        }
    }

    // A valid character literal contains no bytes after the first decoded character.
    // An escape or UTF-8 diagnostic already accounts for a failed decode.
    if (value == -1)
        return value;
    if (cursor != bodyEnd)
    {
        diagnose(sink, token.getLoc(), LexerDiagnostics::illegalCharacterLiteral);
        value = -1;
    }

    return value;
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

/// Returns a raw string's body without decoding escapes, or an empty slice for missing or
/// mismatched delimiters.
///
/// Requires a non-empty `token` whose logical spelling starts with `R"`.
static UnownedStringSlice getRawStringLiteralTokenValue(RawStringLiteral token)
{
    // We check both delimiters in a raw string such as `R"tag(body)tag"` before slicing its body.
    // The lexer can also produce a string token for incomplete text.
    SLANG_RELEASE_ASSERT(token);
    auto content = token.getLogicalSpelling();
    SLANG_RELEASE_ASSERT(content.startsWith("R\""));
    auto delimiterBegin = content.begin() + 2;
    auto openingParenthesis = delimiterBegin;
    while (openingParenthesis != content.end())
    {
        if (*openingParenthesis == '(')
            break;
        ++openingParenthesis;
    }
    if (openingParenthesis == content.end())
        return {};

    // The closing spelling is ')' followed by the opening delimiter and a quote. We first
    // require enough remaining bytes, then compare each part of that closing spelling.
    auto delimiter = UnownedStringSlice(delimiterBegin, openingParenthesis);
    auto bodyBegin = openingParenthesis + 1;
    auto closingLength = delimiter.getLength() + 2;
    if (content.end() - bodyBegin < closingLength)
        return {};
    auto closingBegin = content.end() - closingLength;
    if (*closingBegin != ')')
        return {};
    if (content.end()[-1] != '"')
        return {};
    if (UnownedStringSlice(closingBegin + 1, content.end() - 1) != delimiter)
        return {};
    return UnownedStringSlice(bodyBegin, closingBegin);
}

String getStringLiteralTokenValue(RawStringLiteral token, DiagnosticSink* sink)
{
    // We remove raw-string delimiters without interpreting the body. Ordinary strings
    // instead decode each character or escape between their quotes.
    SLANG_RELEASE_ASSERT(token);
    if (token.getLogicalSpelling().startsWith("R\""))
        return getRawStringLiteralTokenValue(token);

    const UnownedStringSlice content = token.getLogicalSpelling();

    char const* cursor = content.begin();
    char const* end = content.end();

    // The lexer returns a string token even when newline or EOF interrupts the literal.
    // We stop at the end of its content when no closing quote exists, returning the decoded
    // prefix without reading bytes from the following lexeme or beyond the input buffer.
    if (cursor == end)
        return String();

    auto quote = *cursor++;
    SLANG_RELEASE_ASSERT(quote == '"');

    StringBuilder valueBuilder;
    while (cursor < end)
    {
        auto c = *cursor;

        // The lexer ends the token at its closing quote, so no content may follow it.
        if (c == quote)
        {
            ++cursor;
            SLANG_ASSERT(cursor == end);
            return valueBuilder.produceString();
        }

        // Ordinary characters already have their final byte spelling.
        if (c != '\\')
        {
            ++cursor;
            valueBuilder.append(c);
            continue;
        }

        // Byte escapes contribute one code unit; Unicode escapes are encoded as UTF-8.
        bool isUnicodeEscape{};
        IntegerLiteralValue characterValue =
            _decodeStringEscape(cursor, end, sink, token.getLoc(), isUnicodeEscape);

        if (characterValue >= 0)
        {
            if (!isUnicodeEscape)
            {
                if (characterValue <= 255)
                {
                    valueBuilder.append(static_cast<char>(characterValue));
                }
                else
                {
                    String hexadecimalValue;
                    hexadecimalValue.append(characterValue, 16);
                    diagnose(
                        sink,
                        token.getLoc(),
                        LexerDiagnostics::outOfRangeCodeUnit,
                        characterValue,
                        hexadecimalValue.getBuffer());
                }
            }
            else
            {
                char utf8Bytes[4]{};
                size_t byteCount =
                    encodeUnicodePointToUTF8(static_cast<Char32>(characterValue), utf8Bytes);
                SLANG_ASSERT(byteCount <= 4);

                if (byteCount >= 1)
                {
                    valueBuilder.append(utf8Bytes, byteCount);
                }
                else
                {
                    String hexadecimalValue;
                    hexadecimalValue.append(characterValue, 16);
                    diagnose(
                        sink,
                        token.getLoc(),
                        LexerDiagnostics::outOfRangeCodePointForUtf8,
                        characterValue,
                        hexadecimalValue.getBuffer());
                }
            }
        }
    }

    return valueBuilder.produceString();
}

String getFileNameTokenValue(Token const& token)
{
    const UnownedStringSlice content = token.getLogicalSpelling();

    // A file name usually doesn't process escape sequences
    // (this is import on Windows, where `\\` is a valid
    // path separator character).

    // Just trim off the first and last characters to remove the quotes
    // (whether they were `""` or `<>`.
    if (content.getLength() < 2)
        return String();
    return String(content.begin() + 1, content.end() - 1);
}

// Angle-bracket-enclosed header names use preprocessing spelling rules: slashes and quotes inside
// an angle-delimited name are ordinary path characters, and no physical newline may terminate a
// valid name. The caller selects this context before reading '<'.
static TokenType _readAngleBracketHeaderName(Lexer* lexer)
{
    _advance(lexer);
    for (;;)
    {
        const auto next = _peek(lexer);
        if (next == '>')
        {
            _advance(lexer);
            return TokenType::AngleBracketHeaderName;
        }
        if (next == kEOF)
        {
            diagnose(
                lexer->getDiagnosticSink(),
                _getSourceLoc(lexer),
                LexerDiagnostics::endOfFileInLiteral);
            return TokenType::Invalid;
        }
        if (next == '\r' || next == '\n')
        {
            diagnose(
                lexer->getDiagnosticSink(),
                _getSourceLoc(lexer),
                LexerDiagnostics::newlineInLiteral);
            return TokenType::Invalid;
        }
        _advance(lexer);
    }
}

static LexemeTypeTag _readLexemeTypeTag(Lexer* lexer)
{
    int nextCodePoint = _peek(lexer);
    if (lexer->m_mode == LexerMode::ExpectingAngleBracketHeaderName)
    {
        if (nextCodePoint == '<')
            return _readAngleBracketHeaderName(lexer);
    }
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

// A splice beginning at an atomic boundary is its own trivia lexeme. Looking
// only at raw bytes distinguishes that boundary from the character after it.
static bool _isAtStartOfEscapedNewLine(const Lexer* lexer)
{
    if (_peekRawByte(lexer) != '\\')
        return false;
    int next = _peekRawByte(lexer, 1);
    return next == '\r' || next == '\n';
}

// We identify trivia without invoking the token recognizer. In particular,
// looking for trivia after "include" must leave an adjacent '<' untouched so
// the preprocessor can select angle-bracket header recognition before reading that operand.
static bool _isAtStartOfTrivia(Lexer* lexer)
{
    if (_isAtStartOfEscapedNewLine(lexer))
        return true;
    const int c = _peek(lexer);
    switch (c)
    {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        return true;
    case '/':
        {
            const int next = _peek(lexer, 1);
            if (next == '/')
                return true;
            return next == '*';
        }
    default:
        break;
    }
    return false;
}

// We measure an escaped newline starting at `cursor`, or return zero if none starts there.
// A backslash followed by CRLF or LFCR occupies three bytes; one followed by a lone
// CR or LF occupies two. The caller supplies a nonempty remaining physical span.
static Index _getEscapedNewLineLength(const char* cursor, const char* end)
{
    SLANG_ASSERT(cursor < end);
    if (*cursor != '\\')
        return 0;
    if (cursor + 1 == end)
        return 0;

    const auto newline = cursor[1];
    if (newline != '\r')
    {
        if (newline != '\n')
            return 0;
    }

    if (end - cursor > 2)
    {
        if ((newline ^ cursor[2]) == ('\r' ^ '\n'))
            return 3;
    }
    return 2;
}

UnownedStringSlice Lexer::_getScrubbedLogicalSpelling(UnownedStringSlice physicalSpelling)
{
    SLANG_ASSERT(physicalSpelling.getLength() > 0);
    // Scrubbing can only shorten the spelling. Reserving its physical length
    // lets us write the complete logical spelling without further growth checks.
    const auto requiredCapacity = size_t(physicalSpelling.getLength());
    const auto oldCapacity = m_scrubbingBuffer.getCapacityInBytes();
    if (requiredCapacity > oldCapacity)
    {
        auto capacity = requiredCapacity;
        // Geometric growth amortizes allocations across longer lexemes. If
        // doubling would overflow, the exact required capacity is sufficient.
        if (oldCapacity <= std::numeric_limits<size_t>::max() / 2)
        {
            if (capacity < oldCapacity * 2)
                capacity = oldCapacity * 2;
        }
        // Earlier results have already been copied into their pools.
        m_scrubbingBuffer.allocate(capacity);
        SLANG_RELEASE_ASSERT(m_scrubbingBuffer.getData());
    }

    auto begin = static_cast<char*>(m_scrubbingBuffer.getData());
    auto writeCursor = begin;
    for (auto cursor = physicalSpelling.begin(); cursor < physicalSpelling.end();)
    {
        const auto spliceLength = _getEscapedNewLineLength(cursor, physicalSpelling.end());
        if (spliceLength)
        {
            cursor += spliceLength;
            continue;
        }
        *writeCursor++ = *cursor++;
    }

    // The caller supplies an explicit-length view to the pool, which copies it
    // before another read can overwrite these bytes. No terminator is needed here.
    return UnownedStringSlice(begin, writeCursor);
}

Lexeme Lexer::readLexeme()
{
    // Recognition uses logical characters, but we retain the complete byte span
    // consumed from the `SourceView`. A splice at a lexeme boundary is its own
    // trivia item; splices inside a token remain part of that token's physical spelling.
    m_hasLineSpliceInCurrentLexeme = false;
    const auto loc = _getSourceLoc(this);
    const auto textBegin = m_cursor;
    auto kind = TokenType::Count;
    if (_isAtStartOfEscapedNewLine(this))
    {
        _readRawByte(this);
        int newline = _readRawByte(this);
        _handleNewLineInner(this, newline);
        kind = TokenType::EscapedNewLine;
    }
    if (kind == TokenType::Count)
        kind = _readLexemeTypeTag(this);

    const auto flags = m_tokenFlags;
    switch (kind)
    {
    case TokenType::NewLine:
        m_tokenFlags = TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace;
        break;
    case TokenType::WhiteSpace:
    case TokenType::BlockComment:
    case TokenType::LineComment:
        m_tokenFlags |= TokenFlag::AfterWhitespace;
        break;
    case TokenType::EscapedNewLine:
        // A splice disappears from the logical input, so it changes neither
        // line-start context nor the whitespace separating neighboring tokens.
        break;
    default:
        m_tokenFlags = 0;
        break;
    }

    // A boundary splice has empty logical spelling. Internal splices use the
    // lexer's scratch buffer; ordinary lexemes use their physical bytes directly.
    const UnownedStringSlice physicalSpelling(textBegin, m_cursor);
    auto logicalSpelling = physicalSpelling;
    if (kind == TokenType::EscapedNewLine)
        logicalSpelling = UnownedStringSlice();
    else if (m_hasLineSpliceInCurrentLexeme)
        logicalSpelling = _getScrubbedLogicalSpelling(physicalSpelling);

    Name* name = nullptr;
    if (m_namePool)
    {
        if (kind == TokenType::Identifier || kind == TokenType::CompletionRequest)
            name = m_namePool->getName(logicalSpelling);
    }
    return m_tokenPool->createLexeme(kind, physicalSpelling, logicalSpelling, loc, flags, name);
}

Token Lexer::readToken()
{
    // Each trivia item belongs to one cluster. Leading trivia includes blank
    // lines left after the preceding token's first trailing newline; EOF is a
    // normal zero-width token that can own that remaining trivia.

    // We reuse the pointer buffers across reads. The pool copies their
    // contents on a miss before the next read can overwrite the buffers.
    m_leadingTrivia.clear();
    m_trailingTrivia.clear();
    auto& leading = m_leadingTrivia;
    while (_isAtStartOfTrivia(this))
        leading.add(readLexeme().getObject());
    const auto rawToken = readLexeme().as<RawToken>();
    SLANG_RELEASE_ASSERT(rawToken);

    // We stop immediately after the first explicit newline. Embedded newlines
    // do not split a block comment, and escaped newlines do not end directives.
    auto& trailing = m_trailingTrivia;
    if (rawToken.getTypeTag() != TokenType::EndOfFile)
    {
        while (_isAtStartOfTrivia(this))
        {
            auto trivia = readLexeme();
            trailing.add(trivia.getObject());
            if (trivia.getTypeTag() == TokenType::NewLine)
                break;
        }
    }
    return m_tokenPool->createToken(rawToken, leading.getArrayView(), trailing.getArrayView());
}

TokenList Lexer::readAllTokens()
{
    TokenList tokens(m_tokenPool);
    for (;;)
    {
        auto token = readToken();
        tokens.add(token);
        if (token.getTypeTag() == TokenType::EndOfFile)
            return tokens;
    }
}

LexemeList Lexer::readAllLexemes()
{
    LexemeList lexemes(m_tokenPool);
    for (;;)
    {
        auto lexeme = readLexeme();
        lexemes.add(lexeme);
        if (lexeme.getTypeTag() == TokenType::EndOfFile)
            return lexemes;
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

    NamePool namePool;

    RefPtr<TokenPool> tokenPool = new TokenPool();
    lexer.initialize(sourceView, &sink, &namePool, tokenPool);

    auto lexeme = lexer.readLexeme();

    if (lexeme.getTypeTag() == TokenType::Invalid)
    {
        return UnownedStringSlice();
    }

    const int offset = sourceView->getRange().getOffset(lexeme.getLoc());

    if (offset < 0 || offset >= in.getLength())
        return UnownedStringSlice();
    if (Index(offset + lexeme.getPhysicalSpellingLength()) > in.getLength())
        return UnownedStringSlice();

    return UnownedStringSlice(
        in.begin() + offset,
        in.begin() + offset + lexeme.getPhysicalSpellingLength());
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
