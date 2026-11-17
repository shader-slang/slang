// These tests check scanner recognition separately from the agreement between atomic lexemes
// and tokens with attached trivia. We first give each scanner-produced kind an explicit example,
// then check overlapping punctuation and contextual header names. Finally, we use a bounded
// corpus to check byte preservation, locations, progress, and EOF across both lexer interfaces.

#include "compiler-core/slang-lexer.h"
#include "compiler-core/slang-token.h"
#include "core/slang-blob.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

/// A lexer over a borrowed byte range, with a diagnostic location range and an owning token pool.
///
/// The bytes must outlive the fixture, and file decoding must preserve them unchanged.
/// The range need not end with a zero byte.
struct LexicalCoverageInput
{
    SourceManager sourceManager;
    NamePool namePool;
    DiagnosticSink sink;
    RefPtr<TokenPool> tokenPool = new TokenPool();
    Lexer lexer;
    SourceLoc beginLoc;

    explicit LexicalCoverageInput(UnownedStringSlice text)
        : sink(&sourceManager, nullptr)
    {
        // We borrow the exact range so recognition cannot rely on a terminator after the view.
        // File decoding must leave the intended bytes unchanged before the lexer sees them.
        sourceManager.initialize(nullptr, nullptr);
        auto blob = UnownedRawBlob::create(text.begin(), size_t(text.getLength()));
        auto file = sourceManager.createSourceFileWithBlob(PathInfo::makeUnknown(), blob);
        auto view = sourceManager.createSourceView(file, nullptr, SourceLoc());
        SLANG_CHECK_ABORT(view->getContent() == text);
        beginLoc = view->getRange().begin;
        lexer.initialize(view, &sink, &namePool, tokenPool);
    }
};

/// An independently specified spelling and the kind the scanner must recognize from it.
struct ExpectedLexeme
{
    const char* spelling;
    LexemeTypeTag typeTag;
};

/// Checks that lexemes partition the complete input bytes at consecutive source locations.
void _checkPhysicalPartition(
    UnownedStringSlice text,
    SourceLoc beginLoc,
    const List<Lexeme>& lexemes)
{
    // Every non-EOF lexeme must consume bytes. Together with the final EOF, that bounds the
    // number of lexemes by the byte count plus one, even for malformed input.
    SLANG_CHECK_ABORT(lexemes.getCount() > 0);
    SLANG_CHECK_ABORT(lexemes.getCount() <= text.getLength() + 1);
    SLANG_CHECK_ABORT(lexemes.getLast().is<RawEndOfFile>());

    Index offset = 0;
    StringBuilder reconstructed;
    for (Index index = 0; index < lexemes.getCount(); ++index)
    {
        auto lexeme = lexemes[index];
        auto spelling = lexeme.getPhysicalSpelling();
        if (lexeme.is<RawEndOfFile>())
        {
            SLANG_CHECK_ABORT(index == lexemes.getCount() - 1);
            SLANG_CHECK_ABORT(spelling.getLength() == 0);
        }
        else
        {
            SLANG_CHECK_ABORT(spelling.getLength() > 0);
        }

        // We check bounds before slicing so a broken length reports a test failure rather
        // than making the test itself read beyond the borrowed range.
        SLANG_CHECK_ABORT(offset <= text.getLength());
        SLANG_CHECK_ABORT(spelling.getLength() <= text.getLength() - offset);
        SLANG_CHECK_ABORT(lexeme.getLoc() == beginLoc + offset);
        SLANG_CHECK_ABORT(
            spelling == UnownedStringSlice(text.begin() + offset, spelling.getLength()));
        reconstructed << spelling;
        offset += spelling.getLength();
    }
    SLANG_CHECK_ABORT(offset == text.getLength());
    SLANG_CHECK_ABORT(reconstructed.getUnownedSlice() == text);
}

/// Checks that atomic scanning consumes bytes on every read until it reaches the view's end.
void _checkIncrementalProgress(Lexer& lexer)
{
    // A positive byte count per non-EOF result permits at most one read per byte, plus EOF.
    // We bound the loop explicitly so a scanner that returns empty lexemes cannot hang the test.
    Index maximumReads = lexer.m_end - lexer.m_begin + 1;
    for (Index read = 0; read < maximumReads; ++read)
    {
        auto previousCursor = lexer.m_cursor;
        auto lexeme = lexer.readLexeme();
        if (lexeme.is<RawEndOfFile>())
        {
            SLANG_CHECK_ABORT(lexer.m_cursor == lexer.m_end);
            return;
        }
        SLANG_CHECK_ABORT(lexer.m_cursor > previousCursor);
        SLANG_CHECK_ABORT(lexer.m_cursor <= lexer.m_end);
    }
    SLANG_CHECK_ABORT(false);
}

/// Checks that further lexer reads return empty EOF spellings at the same final location.
///
/// Requires the cursor to have reached the end of the view.
void _checkLexerEnd(Lexer& lexer, SourceLoc endLoc)
{
    // The cursor is at the view's end. We check both interfaces without requiring the
    // original EOF's occurrence flags or attached trivia, which belong to that first read.
    SLANG_CHECK_ABORT(lexer.m_cursor == lexer.m_end);
    for (int read = 0; read < 3; ++read)
    {
        auto lexeme = lexer.readLexeme();
        SLANG_CHECK_ABORT(lexeme.is<RawEndOfFile>());
        SLANG_CHECK_ABORT(lexeme.getLoc() == endLoc);
        SLANG_CHECK_ABORT(lexeme.getPhysicalSpelling().getLength() == 0);
        SLANG_CHECK_ABORT(lexeme.getLogicalSpelling().getLength() == 0);
        auto token = lexer.readToken();
        SLANG_CHECK_ABORT(token.is<EndOfFileToken>());
        SLANG_CHECK_ABORT(token.getLoc() == endLoc);
        SLANG_CHECK_ABORT(token.getPhysicalSpelling().getLength() == 0);
        SLANG_CHECK_ABORT(token.getLogicalSpelling().getLength() == 0);
        SLANG_CHECK_ABORT(token.getLeadingTrivia().getCount() == 0);
        SLANG_CHECK_ABORT(token.getTrailingTrivia().getCount() == 0);
        SLANG_CHECK_ABORT(lexer.m_cursor == lexer.m_end);
    }
}

/// Checks that bounded atomic and attached-trivia scans preserve the same complete byte range.
void _checkLexicalRoundTrip(UnownedStringSlice text)
{
    // A nonzero byte immediately after the view makes accidental reliance on a C-string
    // terminator observable. In particular, reading past a final identifier would add `z`.
    StringBuilder storage;
    storage << text;
    storage.appendChar('z');
    UnownedStringSlice boundedText(storage.getBuffer(), text.getLength());

    try
    {
        // We establish positive progress with bounded individual reads before invoking the
        // convenience routines, whose internal loops run until EOF.
        LexicalCoverageInput incrementalInput(boundedText);
        _checkIncrementalProgress(incrementalInput.lexer);
        _checkLexerEnd(incrementalInput.lexer, incrementalInput.beginLoc + text.getLength());

        LexicalCoverageInput atomicInput(boundedText);
        auto atomic = atomicInput.lexer.readAllLexemes();
        auto viewText = atomicInput.lexer.m_sourceView->getContent();
        SLANG_CHECK_ABORT(viewText == boundedText);
        _checkPhysicalPartition(viewText, atomicInput.beginLoc, atomic.getLexemes());

        // For example, when scanning `a\n\n/*c*/b`, `readToken()` attaches one newline
        // after `a` and the remaining trivia before `b`. Flattening every cluster must
        // recover the atomic order.
        LexicalCoverageInput tokenInput(boundedText);
        auto tokens = tokenInput.lexer.readAllTokens();
        List<Lexeme> flattened;
        for (auto token : tokens.getTokens())
        {
            for (auto trivia : token.getLeadingTrivia())
                flattened.add(trivia);
            flattened.add(token.getRawToken());
            for (auto trivia : token.getTrailingTrivia())
                flattened.add(trivia);
        }
        _checkPhysicalPartition(viewText, tokenInput.beginLoc, flattened);
        SLANG_CHECK_ABORT(flattened.getCount() == atomic.getLexemes().getCount());
        SLANG_CHECK_ABORT(atomicInput.sink.getErrorCount() == tokenInput.sink.getErrorCount());

        // Each fixture has its own source manager. We compare offsets within the views,
        // rather than relying on their absolute location ranges being allocated alike.
        for (Index index = 0; index < flattened.getCount(); ++index)
        {
            auto expected = atomic.getLexemes()[index];
            auto actual = flattened[index];
            SLANG_CHECK_ABORT(actual.getTypeTag() == expected.getTypeTag());
            SLANG_CHECK_ABORT(actual.getPhysicalSpelling() == expected.getPhysicalSpelling());
            SLANG_CHECK_ABORT(actual.getLogicalSpelling() == expected.getLogicalSpelling());
            SLANG_CHECK_ABORT(
                actual.getLoc().getRaw() - tokenInput.beginLoc.getRaw() ==
                expected.getLoc().getRaw() - atomicInput.beginLoc.getRaw());

            // Attached trivia has zero occurrence flags. Raw tokens preserve the flags
            // produced by atomic scanning, including line and whitespace context.
            if (expected.is<RawToken>())
            {
                SLANG_CHECK_ABORT(actual.getFlags() == expected.getFlags());
            }
            else
            {
                SLANG_CHECK_ABORT(actual.getFlags() == 0);
            }
        }
        _checkLexerEnd(atomicInput.lexer, atomicInput.beginLoc + text.getLength());
        _checkLexerEnd(tokenInput.lexer, tokenInput.beginLoc + text.getLength());
    }
    catch (AbortTestException&)
    {
        // A corpus failure needs the exact bytes, including zero and malformed UTF-8 bytes,
        // so the failing sample can be reproduced independently of its position in the loops.
        StringBuilder message;
        message << "Lexer input bytes:";
        for (auto byte : text)
            message << " " << String((unsigned char)byte, 16);
        getTestReporter()->message(TestMessageType::Info, message.getBuffer());
        throw;
    }
}

} // namespace

SLANG_UNIT_TEST(lexerRecognizesEveryScannerProducedKind)
{
    // These expected kinds are written independently of the generated lexeme descriptors.
    // A future concrete kind needs a recognition example or an explicit contextual/synthetic
    // classification below; merely adding it to the schema does not extend this oracle.
    const ExpectedLexeme cases[] = {
        {"", LexemeTypeTag::EndOfFile},
        {"`", LexemeTypeTag::Invalid},
        {"name_1", LexemeTypeTag::Identifier},
        {"123u", LexemeTypeTag::IntegerLiteral},
        {"1.25e-2f", LexemeTypeTag::FloatingPointLiteral},
        {"\"text\\n\"", LexemeTypeTag::StringLiteral},
        {"'x'", LexemeTypeTag::CharLiteral},
        {" \t", LexemeTypeTag::WhiteSpace},
        {"\n", LexemeTypeTag::NewLine},
        {"//comment", LexemeTypeTag::LineComment},
        {"/*comment*/", LexemeTypeTag::BlockComment},
        {"\\\n", LexemeTypeTag::EscapedNewLine},
        {";", LexemeTypeTag::Semicolon},
        {",", LexemeTypeTag::Comma},
        {".", LexemeTypeTag::Dot},
        {"..", LexemeTypeTag::DotDot},
        {"...", LexemeTypeTag::Ellipsis},
        {"{", LexemeTypeTag::LBrace},
        {"}", LexemeTypeTag::RBrace},
        {"[", LexemeTypeTag::LBracket},
        {"]", LexemeTypeTag::RBracket},
        {"(", LexemeTypeTag::LParent},
        {")", LexemeTypeTag::RParent},
        {"=", LexemeTypeTag::OpAssign},
        {"+", LexemeTypeTag::OpAdd},
        {"-", LexemeTypeTag::OpSub},
        {"*", LexemeTypeTag::OpMul},
        {"/", LexemeTypeTag::OpDiv},
        {"%", LexemeTypeTag::OpMod},
        {"!", LexemeTypeTag::OpNot},
        {"~", LexemeTypeTag::OpBitNot},
        {"<<", LexemeTypeTag::OpLsh},
        {">>", LexemeTypeTag::OpRsh},
        {"==", LexemeTypeTag::OpEql},
        {"!=", LexemeTypeTag::OpNeq},
        {">", LexemeTypeTag::OpGreater},
        {"<", LexemeTypeTag::OpLess},
        {">=", LexemeTypeTag::OpGeq},
        {"<=", LexemeTypeTag::OpLeq},
        {"&&", LexemeTypeTag::OpAnd},
        {"||", LexemeTypeTag::OpOr},
        {"&", LexemeTypeTag::OpBitAnd},
        {"|", LexemeTypeTag::OpBitOr},
        {"^", LexemeTypeTag::OpBitXor},
        {"++", LexemeTypeTag::OpInc},
        {"--", LexemeTypeTag::OpDec},
        {"+=", LexemeTypeTag::OpAddAssign},
        {"-=", LexemeTypeTag::OpSubAssign},
        {"*=", LexemeTypeTag::OpMulAssign},
        {"/=", LexemeTypeTag::OpDivAssign},
        {"%=", LexemeTypeTag::OpModAssign},
        {"<<=", LexemeTypeTag::OpShlAssign},
        {">>=", LexemeTypeTag::OpShrAssign},
        {"&=", LexemeTypeTag::OpAndAssign},
        {"|=", LexemeTypeTag::OpOrAssign},
        {"^=", LexemeTypeTag::OpXorAssign},
        {"?", LexemeTypeTag::QuestionMark},
        {":", LexemeTypeTag::Colon},
        {"->", LexemeTypeTag::RightArrow},
        {"=>", LexemeTypeTag::DoubleRightArrow},
        {"@", LexemeTypeTag::At},
        {"$", LexemeTypeTag::Dollar},
        {"$$", LexemeTypeTag::DollarDollar},
        {"#", LexemeTypeTag::Pound},
        {"##", LexemeTypeTag::PoundPound},
        {"::", LexemeTypeTag::Scope},
        {"#?", LexemeTypeTag::CompletionRequest},
    };
    bool coveredTags[size_t(LexemeTypeTag::Count)] = {};
    for (auto expected : cases)
    {
        UnownedStringSlice spelling(expected.spelling);
        LexicalCoverageInput input(spelling);
        auto lexeme = input.lexer.readLexeme();
        SLANG_CHECK(lexeme.getTypeTag() == expected.typeTag);
        SLANG_CHECK(lexeme.getPhysicalSpelling() == spelling);
        auto logicalSpelling = spelling;
        if (expected.typeTag == LexemeTypeTag::EscapedNewLine)
            logicalSpelling = UnownedStringSlice();
        SLANG_CHECK(lexeme.getLogicalSpelling() == logicalSpelling);
        SLANG_CHECK(lexeme.getLoc() == input.beginLoc);
        SLANG_CHECK(input.lexer.readLexeme().is<RawEndOfFile>());
        coveredTags[size_t(expected.typeTag)] = true;
    }

    // `AngleBracketHeaderName` requires an explicit lexer mode and has its own test.
    // The preprocessor synthesizes `EndOfDirective`; the scanner never returns that kind.
    coveredTags[size_t(LexemeTypeTag::AngleBracketHeaderName)] = true;
    coveredTags[size_t(LexemeTypeTag::EndOfDirective)] = true;
    for (bool isCovered : coveredTags)
    {
        SLANG_CHECK(isCovered);
    }
}

SLANG_UNIT_TEST(lexerPunctuationUsesLongestSpelling)
{
    // Each run has more punctuation than one token can consume. We specify the complete
    // segmentation, including the final shorter token, so accepting a prefix cannot pass.
    const char* text = "<<=<>>=>===+++---=>&&&|||^^=##?#?$$$::::....";
    const ExpectedLexeme expected[] = {
        {"<<=", LexemeTypeTag::OpShlAssign},
        {"<", LexemeTypeTag::OpLess},
        {">>=", LexemeTypeTag::OpShrAssign},
        {">=", LexemeTypeTag::OpGeq},
        {"==", LexemeTypeTag::OpEql},
        {"++", LexemeTypeTag::OpInc},
        {"+", LexemeTypeTag::OpAdd},
        {"--", LexemeTypeTag::OpDec},
        {"-=", LexemeTypeTag::OpSubAssign},
        {">", LexemeTypeTag::OpGreater},
        {"&&", LexemeTypeTag::OpAnd},
        {"&", LexemeTypeTag::OpBitAnd},
        {"||", LexemeTypeTag::OpOr},
        {"|", LexemeTypeTag::OpBitOr},
        {"^", LexemeTypeTag::OpBitXor},
        {"^=", LexemeTypeTag::OpXorAssign},
        {"##", LexemeTypeTag::PoundPound},
        {"?", LexemeTypeTag::QuestionMark},
        {"#?", LexemeTypeTag::CompletionRequest},
        {"$$", LexemeTypeTag::DollarDollar},
        {"$", LexemeTypeTag::Dollar},
        {"::", LexemeTypeTag::Scope},
        {"::", LexemeTypeTag::Scope},
        {"...", LexemeTypeTag::Ellipsis},
        {".", LexemeTypeTag::Dot},
    };
    LexicalCoverageInput input{UnownedStringSlice(text)};
    for (auto lexeme : expected)
    {
        auto actual = input.lexer.readLexeme();
        SLANG_CHECK(actual.getTypeTag() == lexeme.typeTag);
        SLANG_CHECK(actual.getPhysicalSpelling() == UnownedStringSlice(lexeme.spelling));
    }
    SLANG_CHECK(input.lexer.readLexeme().is<RawEndOfFile>());
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

SLANG_UNIT_TEST(lexerHeaderNameRecognitionIsContextual)
{
    // The same `<` begins punctuation normally and a complete header name in the selected
    // mode. Comment-looking bytes belong to the header name rather than separate trivia.
    const auto text = UnownedStringSlice("<one/*two*/.h>");
    LexicalCoverageInput normal(text);
    SLANG_CHECK(normal.lexer.readLexeme().is<RawOpLess>());
    LexicalCoverageInput header(text);
    header.lexer.setMode(LexerMode::ExpectingAngleBracketHeaderName);
    auto name = header.lexer.readLexeme();
    SLANG_CHECK(name.is<RawAngleBracketHeaderName>());
    SLANG_CHECK(name.getPhysicalSpelling() == text);
    SLANG_CHECK(name.getLogicalSpelling() == text);
    SLANG_CHECK(header.lexer.readLexeme().is<RawEndOfFile>());
    SLANG_CHECK(header.sink.getErrorCount() == 0);
}

SLANG_UNIT_TEST(lexerEmbeddedZeroRetainsTheRemainingPhysicalBytes)
{
    // We need a zero byte to reach the lexer unchanged. `SourceFile::decodeContentBlob()`
    // can infer UTF-16 from zero bytes in an even-length buffer. We use odd-length inputs
    // below, and the fixture checks that file decoding leaves their bytes unchanged.
    const char bytes[] = {'a', 0, 't', 'a', 'i', 'l', '!', 'z'};
    const auto text = UnownedStringSlice(bytes, size_t(7));
    LexicalCoverageInput input(text);

    // `_readCodePoint()` returns zero and moves the cursor to the view's end. At an atomic
    // boundary, the scanner therefore returns `Invalid` for the entire remaining byte range.
    SLANG_CHECK(input.lexer.readLexeme().is<RawIdentifier>());
    auto invalid = input.lexer.readLexeme();
    SLANG_CHECK(invalid.is<RawInvalid>());
    SLANG_CHECK(invalid.getPhysicalSpelling() == UnownedStringSlice(bytes + 1, size_t(6)));
    SLANG_CHECK(invalid.getLogicalSpelling() == invalid.getPhysicalSpelling());
    SLANG_CHECK(invalid.getLoc() == input.beginLoc + 1);
    SLANG_CHECK(input.sink.getErrorCount() == 1);
    _checkLexerEnd(input.lexer, input.beginLoc + text.getLength());
    _checkLexicalRoundTrip(text);

    // Within a comment or literal, the scanner consumes zero as part of that construct and
    // next observes EOF. Bytes after zero remain in its physical spelling, even when those
    // bytes include a closing delimiter and text that would otherwise become another token.
    const ExpectedLexeme constructs[] = {
        {"//", LexemeTypeTag::LineComment},
        {"/*", LexemeTypeTag::BlockComment},
        {"\"", LexemeTypeTag::StringLiteral},
        {"'", LexemeTypeTag::CharLiteral},
        {"R\"(", LexemeTypeTag::StringLiteral},
    };
    for (auto construct : constructs)
    {
        StringBuilder storage;
        storage << construct.spelling;
        storage.appendChar(0);
        storage << "*/\"')\"tail";
        // The assembled construct also needs an odd byte count for UTF-8 detection.
        if (storage.getLength() % 2 == 0)
            storage.appendChar('!');
        auto constructText = storage.getUnownedSlice();
        LexicalCoverageInput embedded(constructText);
        auto lexeme = embedded.lexer.readLexeme();
        SLANG_CHECK(lexeme.getTypeTag() == construct.typeTag);
        SLANG_CHECK(lexeme.getPhysicalSpelling() == constructText);
        SLANG_CHECK(lexeme.getLogicalSpelling() == constructText);
        _checkLexerEnd(embedded.lexer, embedded.beginLoc + constructText.getLength());
        _checkLexicalRoundTrip(constructText);
    }
}

SLANG_UNIT_TEST(lexerBoundedFragmentsPreserveBothLexicalStreams)
{
    // Prefixes cut through UTF-8 sequences, comment openers, escapes, numeric exponents, and
    // raw-string delimiters. Ordered pairs exercise interactions at fragment boundaries,
    // including newlines swallowed by comments or removed by splicing. The corpus is fixed.
    // We keep one fragment per line so escaped bytes and truncation cases remain easy to inspect.
    // clang-format off
    const char* fragments[] = {
        "",
        "name",
        " ",
        "\t",
        "\r",
        "\n",
        "\r\n",
        "\n\r",
        "\\\n",
        "a\\\r\nb",
        "//c",
        "/*c*/",
        "/*",
        "\"s\\\"\"",
        "\"",
        "'x'",
        "'",
        "R\"d(body)d\"",
        "R\"d(",
        "0x",
        "1e+",
        ".5",
        "<<=",
        "#?",
        "\\",
        "\xE2\x82",
        "\xED\xA0\x80",
        "\x80",
    };
    // clang-format on
    for (auto fragment : fragments)
    {
        auto text = UnownedStringSlice(fragment);
        for (Index length = 0; length <= text.getLength(); ++length)
            _checkLexicalRoundTrip(UnownedStringSlice(text.begin(), length));
        for (auto following : fragments)
        {
            StringBuilder combined;
            combined << fragment << following;
            _checkLexicalRoundTrip(combined.getUnownedSlice());
        }
    }
}
