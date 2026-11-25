// These tests exercise the contracts between atomic lexing, trivia attachment, and borrowed
// token values. We first check interning and ownership without a lexer, then reconstruct
// physical input and exercise the boundaries where preprocessing changes lexer state.
// We allocate pools under `RefPtr` because `TokenList` and `TokenRef` retain pool references.

#include "compiler-core/slang-lexer.h"
#include "compiler-core/slang-token.h"
#include "core/slang-blob.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

/// A lexer and diagnostic locations for one borrowed UTF-8 input buffer.
///
/// The input buffer must outlive the fixture; a null terminator is not required.
struct LexemeTestInput
{
    SourceManager sourceManager;
    NamePool namePool;
    DiagnosticSink sink;
    RefPtr<TokenPool> tokenPool = new TokenPool();
    Lexer lexer;
    SourceLoc beginLoc;

    explicit LexemeTestInput(const char* text)
        : LexemeTestInput(UnownedStringSlice(text))
    {
    }

    explicit LexemeTestInput(UnownedStringSlice text)
        : sink(&sourceManager, nullptr)
    {
        // We keep the file and its view together so byte offsets in the assertions refer
        // to the same location range that the lexer uses.
        sourceManager.initialize(nullptr, nullptr);
        auto blob = UnownedRawBlob::create(text.begin(), size_t(text.getLength()));
        auto file = sourceManager.createSourceFileWithBlob(PathInfo::makeUnknown(), blob);
        auto view = sourceManager.createSourceView(file, nullptr, SourceLoc());
        beginLoc = view->getRange().begin;
        lexer.initialize(view, &sink, &namePool, tokenPool);
    }
};

/// A token pool that sets a supplied flag when its final owning reference is released.
///
/// The flag must outlive the pool.
struct TokenPoolWithDestructionFlag : TokenPool
{
    explicit TokenPoolWithDestructionFlag(bool& wasDestroyed)
        : m_wasDestroyed(wasDestroyed)
    {
    }

    ~TokenPoolWithDestructionFlag() override { m_wasDestroyed = true; }

private:
    bool& m_wasDestroyed;
};

} // namespace

/// Checks that interning shares spelling data while occurrence locations and flags stay distinct.
SLANG_UNIT_TEST(lexemePoolSeparatesSpellingFromOccurrences)
{
    // The same spelling can occur at different locations and with different preprocessing
    // context. Only the returned values differ; both refer to one immutable object.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    const auto physicalSpelling = UnownedStringSlice("alpha\\\nbeta");
    const auto logicalSpelling = UnownedStringSlice("alphabeta");
    auto first = pool.createLexeme(
        LexemeTypeTag::Identifier,
        physicalSpelling,
        logicalSpelling,
        SourceLoc::fromRaw(100));
    auto second = pool.createLexeme(
        LexemeTypeTag::Identifier,
        physicalSpelling,
        logicalSpelling,
        SourceLoc::fromRaw(200),
        TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace);
    SLANG_CHECK(first.getObject() == second.getObject());
    SLANG_CHECK(first.getLoc() == SourceLoc::fromRaw(100));
    SLANG_CHECK(second.getLoc() == SourceLoc::fromRaw(200));
    SLANG_CHECK(first.getFlags() == 0);
    SLANG_CHECK(second.getFlags() == (TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace));

    // Identical logical spellings do not make different physical spellings share an object.
    // Keeping that distinction lets a token stream reproduce the escaped newline exactly.
    auto unspliced = pool.createLexeme(LexemeTypeTag::Identifier, logicalSpelling, logicalSpelling);
    SLANG_CHECK(first.getObject() != unspliced.getObject());
    SLANG_CHECK(first.getLogicalSpelling() == unspliced.getLogicalSpelling());
    SLANG_CHECK(first.getPhysicalSpelling() == physicalSpelling);
    SLANG_CHECK(unspliced.getPhysicalSpelling() == logicalSpelling);
}

/// Checks that changing caller-owned text cannot alter a pooled spelling or its interning identity.
SLANG_UNIT_TEST(lexemePoolOwnsInputText)
{
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    char spelling[] = "item";
    auto lexeme = pool.createLexeme(
        LexemeTypeTag::Identifier,
        UnownedStringSlice(spelling),
        UnownedStringSlice(spelling));
    spelling[0] = 'X';
    SLANG_CHECK(lexeme.getPhysicalSpelling() == UnownedStringSlice("item"));
    SLANG_CHECK(lexeme.getLogicalSpelling() == UnownedStringSlice("item"));

    auto repeated = pool.createLexeme(
        LexemeTypeTag::Identifier,
        UnownedStringSlice("item"),
        UnownedStringSlice("item"));
    SLANG_CHECK(lexeme.getObject() == repeated.getObject());
}

/// Checks that token interning preserves trivia order and occurrence-specific locations and flags.
SLANG_UNIT_TEST(tokenPoolInternsOrderedTrivia)
{
    // For the physical cluster " /*c*/x\n", the raw token starts six bytes after
    // its leading trivia. A second occurrence shares objects but retains its own context.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    auto space = pool.createLexeme(
        LexemeTypeTag::WhiteSpace,
        UnownedStringSlice(" "),
        UnownedStringSlice(" "));
    auto comment = pool.createLexeme(
        LexemeTypeTag::BlockComment,
        UnownedStringSlice("/*c*/"),
        UnownedStringSlice("/*c*/"));
    auto newline = pool.createLexeme(
        LexemeTypeTag::NewLine,
        UnownedStringSlice("\n"),
        UnownedStringSlice("\n"));
    const LexemeObj* leading[] = {space.getObject(), comment.getObject()};
    const LexemeObj* trailing[] = {newline.getObject()};
    auto raw = pool.createLexeme(
        LexemeTypeTag::Identifier,
        UnownedStringSlice("x"),
        UnownedStringSlice("x"),
        SourceLoc::fromRaw(100));
    auto token = pool.createToken(
        raw.as<RawToken>(),
        makeConstArrayView(leading),
        makeConstArrayView(trailing));
    auto otherRaw = pool.createLexeme(
        LexemeTypeTag::Identifier,
        UnownedStringSlice("x"),
        UnownedStringSlice("x"),
        SourceLoc::fromRaw(200),
        TokenFlag::AfterWhitespace);
    auto repeated = pool.createToken(
        otherRaw.as<RawToken>(),
        makeConstArrayView(leading),
        makeConstArrayView(trailing));
    SLANG_CHECK(token.getObject() == repeated.getObject());
    SLANG_CHECK(repeated.getLoc() == SourceLoc::fromRaw(200));
    SLANG_CHECK(repeated.getFlags() == TokenFlag::AfterWhitespace);

    // Trivia order and the division into leading and trailing lists are both part of
    // token identity, even if all clusters reference the same individual objects.
    const LexemeObj* reversed[] = {comment.getObject(), space.getObject()};
    auto reordered = pool.createToken(
        raw.as<RawToken>(),
        makeConstArrayView(reversed),
        makeConstArrayView(trailing));
    const LexemeObj* repartitionedTrailing[] = {comment.getObject(), newline.getObject()};
    auto repartitioned = pool.createToken(
        raw.as<RawToken>(),
        makeConstArrayView(leading, 1),
        makeConstArrayView(repartitionedTrailing));
    SLANG_CHECK(token.getObject() != reordered.getObject());
    SLANG_CHECK(token.getObject() != repartitioned.getObject());
    SLANG_CHECK_ABORT(token.getLeadingTrivia().getCount() == 2);
    SLANG_CHECK_ABORT(token.getTrailingTrivia().getCount() == 1);
    SLANG_CHECK(token.getLeadingTrivia()[0].getLoc() == SourceLoc::fromRaw(94));
    SLANG_CHECK(token.getLeadingTrivia()[1].getLoc() == SourceLoc::fromRaw(95));
    SLANG_CHECK(token.getTrailingTrivia()[0].getLoc() == SourceLoc::fromRaw(101));
}

/// Checks that empty checked values differ from EOF and that wrappers preserve type membership.
SLANG_UNIT_TEST(lexemeTypedValuesDistinguishEmptyFromEndOfFile)
{
    // Failed casts return empty values. EOF has an object and remains a valid member
    // of the token hierarchy, despite having no spelling bytes.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    RawIdentifier emptyLexeme;
    IdentifierToken emptyToken;
    SLANG_CHECK(!emptyLexeme);
    SLANG_CHECK(!emptyToken);
    auto eof = pool.createEndOfFile();
    SLANG_CHECK(bool(eof));
    SLANG_CHECK(eof.is<EndOfFileToken>());
    SLANG_CHECK(bool(eof.getRawToken()));
    SLANG_CHECK(!eof.as<IdentifierToken>());

    // Abstract groups and concrete wrappers classify the same objects without creating
    // extra payloads. Numeric literals belong to both numeric and literal groups.
    auto number = pool.createToken(TokenType::IntegerLiteral, UnownedStringSlice("12"));
    SLANG_CHECK(number.is<IntegerLiteralToken>());
    SLANG_CHECK(number.isInstanceOf<LiteralToken>());
    SLANG_CHECK(number.isInstanceOf(LexemeTypes::NumericLiteral));
    SLANG_CHECK(number.is<LiteralToken>());
    SLANG_CHECK(number.is<Token>());
    SLANG_CHECK(number.getRawToken().is<RawNumericLiteral>());
    SLANG_CHECK(number.getRawToken().is<Lexeme>());
    SLANG_CHECK(!number.is<IdentifierToken>());
    SLANG_CHECK(number.as<IntegerLiteralToken>().getObject() == number.getObject());
}

/// Checks that replacement spellings and flags leave shared immutable token objects unchanged.
SLANG_UNIT_TEST(tokenReplacementPreservesSharedObject)
{
    // A replacement is a fresh factory request.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    auto original = pool.createToken(TokenType::Identifier, UnownedStringSlice("before"));
    auto sharedOriginal = original;
    auto replaced = pool.createToken(TokenType::Identifier, UnownedStringSlice("after"));
    SLANG_CHECK(original.getLogicalSpelling() == UnownedStringSlice("before"));
    SLANG_CHECK(sharedOriginal.getObject() == original.getObject());
    SLANG_CHECK(sharedOriginal.getLogicalSpelling() == UnownedStringSlice("before"));
    SLANG_CHECK(replaced.getLogicalSpelling() == UnownedStringSlice("after"));
    SLANG_CHECK(original.getObject() != replaced.getObject());

    auto repeated = pool.createToken(TokenType::Identifier, UnownedStringSlice("after"));
    SLANG_CHECK(replaced.getObject() == repeated.getObject());
    auto flagged = replaced.withFlags(TokenFlag::AfterWhitespace);
    SLANG_CHECK(flagged.getObject() == repeated.getObject());
    SLANG_CHECK(flagged.getFlags() == TokenFlag::AfterWhitespace);
    SLANG_CHECK(replaced.getFlags() == 0);
    SLANG_CHECK(repeated.getFlags() == 0);
}

/// Checks that a conditional read advances only on a match and returns a typed token or empty.
SLANG_UNIT_TEST(tokenReaderTypedConditionalRead)
{
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    TokenList tokens(poolOwner);
    tokens.add(pool.createToken(TokenType::Identifier, UnownedStringSlice("value")));
    tokens.add(pool.createToken(TokenType::IntegerLiteral, UnownedStringSlice("12")));
    tokens.add(pool.createEndOfFile());
    TokenReader reader(tokens);
    auto start = reader.getCursor();
    SLANG_CHECK(!reader.advanceIf<IntegerLiteralToken>());
    SLANG_CHECK(reader.isAtCursor(start));
    auto identifier = reader.advanceIf<IdentifierToken>();
    SLANG_CHECK(bool(identifier));
    SLANG_CHECK(identifier.getLogicalSpelling() == UnownedStringSlice("value"));
    SLANG_CHECK(reader.peekTokenType() == TokenType::IntegerLiteral);

    // A conditional read can use an abstract token group, and a successful EOF read
    // remains distinguishable from the empty result of a mismatched read.
    auto literal = reader.advanceIf<LiteralToken>();
    SLANG_CHECK(literal.getLogicalSpelling() == UnownedStringSlice("12"));
    SLANG_CHECK(bool(reader.advanceIf<EndOfFileToken>()));
    SLANG_CHECK(reader.isAtEnd());
    SLANG_CHECK(!reader.advanceIf<IdentifierToken>());
}

/// Checks that splitting `>>` preserves physical locations, trailing trivia, and unknown locations.
SLANG_UNIT_TEST(tokenSplitPreservesPhysicalOffsetsAndTrailingTrivia)
{
    // The first `>` is followed by a backslash, a newline, and the second `>`.
    // Splitting uses the physical offset of that second character, three bytes after
    // the first one, rather than the offset within the logical spelling `>>`.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    auto rightShift = pool.createLexeme(
        LexemeTypeTag::OpRsh,
        UnownedStringSlice(">\\\n>"),
        UnownedStringSlice(">>"),
        SourceLoc::fromRaw(100),
        TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace);
    auto comment = pool.createLexeme(
        LexemeTypeTag::LineComment,
        UnownedStringSlice("//tail"),
        UnownedStringSlice("//tail"));
    const LexemeObj* trailing[] = {comment.getObject()};
    auto original = pool.createToken(rightShift.as<RawToken>(), {}, makeConstArrayView(trailing));
    auto second = pool.createSecondGreaterToken(rightShift.as<RawOpRsh>());
    SLANG_CHECK(second.getTypeTag() == TokenType::OpGreater);
    SLANG_CHECK(second.getPhysicalSpelling() == UnownedStringSlice(">"));
    SLANG_CHECK(second.getLogicalSpelling() == UnownedStringSlice(">"));
    SLANG_CHECK(second.getLoc() == SourceLoc::fromRaw(103));
    SLANG_CHECK(second.getFlags() == 0);
    SLANG_CHECK(rightShift.getLogicalSpelling() == UnownedStringSlice(">>"));

    // Trivia following the original operator is still adjacent to the second half.
    // Reusing those objects must therefore preserve the comment's physical location.
    auto secondCluster = pool.createToken(second, {}, original.getTrailingTrivia().getObjects());
    SLANG_CHECK_ABORT(secondCluster.getTrailingTrivia().getCount() == 1);
    SLANG_CHECK(secondCluster.getTrailingTrivia()[0].getLoc() == SourceLoc::fromRaw(104));
    SLANG_CHECK(
        secondCluster.getTrailingTrivia()[0].getLoc() == original.getTrailingTrivia()[0].getLoc());

    // Synthesized operators can have no source location. Splitting must keep that
    // absence rather than interpreting byte offset one as a real source location.
    auto unlocated = pool.createToken(TokenType::OpRsh, UnownedStringSlice(">>"));
    SLANG_CHECK(
        !pool.createSecondGreaterToken(unlocated.getRawToken().as<RawOpRsh>()).getLoc().isValid());
}

/// Checks that copied token references retain lexical storage and names until their last release.
SLANG_UNIT_TEST(tokenRefRetainsPoolAndName)
{
    // Clients without an AST can retain a typed token after both the name pool and
    // the original pool reference go away. The token pool must retain its `Name` objects.
    bool wasPoolDestroyed = false;
    bool wasReplacedPoolDestroyed = false;
    TokenRef<IdentifierToken> retained;
    {
        NamePool names;
        RefPtr<TokenPool> pool = new TokenPoolWithDestructionFlag(wasPoolDestroyed);
        auto name = names.getName(String("retained"));
        auto token = pool->createToken(TokenType::Identifier, name).as<IdentifierToken>();
        retained = TokenRef<IdentifierToken>(token);
    }
    SLANG_CHECK_ABORT(!wasPoolDestroyed);
    SLANG_CHECK(bool(retained));
    SLANG_CHECK(retained->getLogicalSpelling() == UnownedStringSlice("retained"));
    SLANG_CHECK_ABORT(retained->getName() != nullptr);
    SLANG_CHECK(retained->getName()->text == "retained");

    // Copying from an rvalue must keep both handles' tokens backed by their pools.
    // We destroy each destination before checking that the original still owns its pool.
    {
        TokenRef<IdentifierToken> copied(_Move(retained));
        SLANG_CHECK(copied->getObject() == retained->getObject());
    }
    SLANG_CHECK_ABORT(!wasPoolDestroyed);
    SLANG_CHECK(retained->getLogicalSpelling() == UnownedStringSlice("retained"));
    {
        RefPtr<TokenPool> replacedPool = new TokenPoolWithDestructionFlag(wasReplacedPoolDestroyed);
        TokenRef<IdentifierToken> assigned(
            replacedPool->createToken(TokenType::Identifier, UnownedStringSlice("replaced"))
                .as<IdentifierToken>());
        replacedPool = nullptr;

        // Assignment releases the destination's former pool instead of transferring
        // that unrelated pool to the original handle.
        assigned = _Move(retained);
        SLANG_CHECK(wasReplacedPoolDestroyed);
        SLANG_CHECK(assigned->getObject() == retained->getObject());
    }
    SLANG_CHECK_ABORT(!wasPoolDestroyed);
    SLANG_CHECK(retained->getLogicalSpelling() == UnownedStringSlice("retained"));
    retained = TokenRef<IdentifierToken>();
    SLANG_CHECK(wasPoolDestroyed);
}

/// Checks that a token list retains its chosen pool through value replacement and releases it
/// when the list is reset.
SLANG_UNIT_TEST(tokenListRetainsItsSelectedPool)
{
    bool wasDestroyed = false;
    TokenList tokens;
    {
        RefPtr<TokenPool> pool = new TokenPoolWithDestructionFlag(wasDestroyed);
        tokens = TokenList(pool);
        tokens.add(pool->createToken(TokenType::Identifier, UnownedStringSlice("one")));
        tokens.add(pool->createEndOfFile());
        SLANG_CHECK(tokens.getTokenPool() == pool.get());
    }
    SLANG_CHECK_ABORT(!wasDestroyed);
    tokens.setTokens(tokens.getTokens());
    {
        TokenReader reader(tokens);
        SLANG_CHECK(
            reader.advanceIf<IdentifierToken>().getLogicalSpelling() == UnownedStringSlice("one"));
        SLANG_CHECK(reader.peekToken().getPool() == tokens.getTokenPool());
    }
    tokens = TokenList();
    SLANG_CHECK(wasDestroyed);
}

/// Checks that a saved token list owns spelling, names, and trivia after its input owners expire.
SLANG_UNIT_TEST(tokenListRetainsLexicalStorage)
{
    // A saved token list can outlive the lexer, the name pool, and the `SourceManager`.
    // Its selected pool must preserve spelling and names without borrowing file bytes.
    TokenList tokens;
    {
        LexemeTestInput input("one //tail\n");
        tokens = input.lexer.readAllTokens();
        SLANG_CHECK(input.sink.getErrorCount() == 0);
    }

    TokenReader reader(tokens);
    auto first = reader.advanceIf<IdentifierToken>();
    SLANG_CHECK(first.getLogicalSpelling() == UnownedStringSlice("one"));
    SLANG_CHECK_ABORT(first.getName() != nullptr);
    SLANG_CHECK(first.getName()->text == "one");
    SLANG_CHECK_ABORT(first.getTrailingTrivia().getCount() == 3);
    SLANG_CHECK(first.getTrailingTrivia()[1].getPhysicalSpelling() == UnownedStringSlice("//tail"));
    SLANG_CHECK(reader.peekTokenType() == TokenType::EndOfFile);
}

/// Checks that appending a token already in the list preserves its occurrence while the list grows.
SLANG_UNIT_TEST(tokenListAppendPreservesAliasedOccurrence)
{
    // Appending an occurrence already in the sequence must remain valid while the
    // sequence grows and relocates its values. The pool and occurrence stay the same.
    RefPtr<TokenPool> pool = new TokenPool();
    TokenList tokens(pool);
    auto first = pool->createToken(
        TokenType::Identifier,
        UnownedStringSlice("kept"),
        SourceLoc::fromRaw(100),
        TokenFlag::AfterWhitespace);
    tokens.add(first);
    for (Index i = 0; i < 128; ++i)
        tokens.add(tokens.getTokens()[0]);
    tokens.add(pool->createEndOfFile());
    for (auto token : tokens)
    {
        SLANG_CHECK(token.getObject() == first.getObject());
        SLANG_CHECK(token.getLoc() == first.getLoc());
        SLANG_CHECK(token.getFlags() == first.getFlags());
    }
}

/// Checks that indexed and sequential trivia access preserve unknown source locations.
SLANG_UNIT_TEST(tokenTriviaPreservesUnknownSourceLocations)
{
    // A token reconstructed without debug locations can still have spelling and
    // trivia. Neither indexing nor iteration may invent locations from text lengths.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    auto raw = pool.createLexeme(
        LexemeTypeTag::IntegerLiteral,
        UnownedStringSlice("42"),
        UnownedStringSlice("42"));
    auto space = pool.createLexeme(
        LexemeTypeTag::WhiteSpace,
        UnownedStringSlice(" "),
        UnownedStringSlice(" "));
    auto comment = pool.createLexeme(
        LexemeTypeTag::BlockComment,
        UnownedStringSlice("/*c*/"),
        UnownedStringSlice("/*c*/"));
    auto newline = pool.createLexeme(
        LexemeTypeTag::NewLine,
        UnownedStringSlice("\n"),
        UnownedStringSlice("\n"));
    const LexemeObj* leading[] = {space.getObject(), comment.getObject()};
    const LexemeObj* trailing[] = {newline.getObject(), space.getObject()};
    auto token = pool.createToken(
        raw.as<RawToken>(),
        makeConstArrayView(leading),
        makeConstArrayView(trailing));
    SLANG_CHECK(!token.getLoc().isValid());
    SLANG_CHECK_ABORT(token.getLeadingTrivia().getCount() == 2);
    SLANG_CHECK_ABORT(token.getTrailingTrivia().getCount() == 2);
    SLANG_CHECK(!token.getLeadingTrivia()[1].getLoc().isValid());
    SLANG_CHECK(!token.getTrailingTrivia()[1].getLoc().isValid());

    StringBuilder reconstructed;
    for (auto trivia : token.getLeadingTrivia())
    {
        SLANG_CHECK(!trivia.getLoc().isValid());
        reconstructed << trivia.getPhysicalSpelling();
    }
    reconstructed << token.getPhysicalSpelling();
    for (auto trivia : token.getTrailingTrivia())
    {
        SLANG_CHECK(!trivia.getLoc().isValid());
        reconstructed << trivia.getPhysicalSpelling();
    }
    SLANG_CHECK(reconstructed.getUnownedSlice() == UnownedStringSlice(" /*c*/42\n "));
}

/// Checks that atomic lexemes and EOF preserve every input byte and its physical location.
SLANG_UNIT_TEST(lexerAtomicLexemesReconstructInput)
{
    // Each spelling byte belongs to one atomic lexeme. Standalone line continuations
    // and escaped newlines inside identifiers must therefore survive independently of
    // the logical spelling used for parsing.
    const char* inputs[] = {
        "",
        " \t// comment\r\n",
        "alpha\\\r\nbeta /*one\n two*/ + 1\r\n",
        "\\\n",
        "\\\r\n",
        "name \\\n + next",
        "name\\\n",
        "name\\\r\n",
        "name \\\n",
        "name \\\r\n",
        ">\\\n",
        ">\\\r\n",
    };
    for (auto text : inputs)
    {
        LexemeTestInput input(text);
        auto lexemes = input.lexer.readAllLexemes();
        SLANG_CHECK_ABORT(lexemes.getLexemes().getCount() > 0);
        StringBuilder reconstructed;
        Index offset = 0;
        for (auto lexeme : lexemes)
        {
            SLANG_CHECK(lexeme.getLoc() == input.beginLoc + offset);
            reconstructed << lexeme.getPhysicalSpelling();
            offset += lexeme.getPhysicalSpellingLength();
        }
        SLANG_CHECK(lexemes.end()->is<RawEndOfFile>());
        SLANG_CHECK(lexemes.end()->getLoc() == input.beginLoc + offset);
        SLANG_CHECK(reconstructed.getUnownedSlice() == UnownedStringSlice(text));
        SLANG_CHECK(input.sink.getErrorCount() == 0);
    }
}

/// Checks that a final backslash is diagnosed without reading outside a bounded input view.
SLANG_UNIT_TEST(lexerFinalBackslashUsesBufferEnd)
{
    // The byte after the view is deliberately nonzero. The final backslash is an
    // invalid character, and the lexer must not treat the excluded byte as input.
    const char bytes[] = {'\\', 'x'};
    LexemeTestInput input(UnownedStringSlice(bytes, size_t(1)));
    SLANG_CHECK(input.lexer.m_end == bytes + 1);
    auto backslash = input.lexer.readLexeme();
    SLANG_CHECK(backslash.is<RawInvalid>());
    SLANG_CHECK(backslash.getPhysicalSpelling() == UnownedStringSlice("\\"));
    auto eof = input.lexer.readLexeme();
    SLANG_CHECK(eof.is<RawEndOfFile>());
    SLANG_CHECK(eof.getLoc() == input.beginLoc + 1);
    SLANG_CHECK(input.sink.getErrorCount() == 1);
}

/// Checks that a terminal splice preserves a complete CRLF or LFCR within the bounded view.
SLANG_UNIT_TEST(lexerTerminalSpliceUsesCompleteNewline)
{
    // A terminal splice is trivia after `a`. Both two-byte newline spellings must
    // remain intact, and the nonzero byte outside each view must remain unread.
    const char buffers[][5] = {{'a', '\\', '\r', '\n', 'x'}, {'a', '\\', '\n', '\r', 'x'}};
    for (const auto& bytes : buffers)
    {
        LexemeTestInput input(UnownedStringSlice(bytes, size_t(4)));
        auto identifier = input.lexer.readLexeme();
        SLANG_CHECK(identifier.is<RawIdentifier>());
        SLANG_CHECK(identifier.getPhysicalSpelling() == UnownedStringSlice("a"));
        auto splice = input.lexer.readLexeme();
        SLANG_CHECK(splice.is<EscapedNewLineTrivia>());
        SLANG_CHECK(splice.getPhysicalSpelling() == UnownedStringSlice(bytes + 1, bytes + 4));
        SLANG_CHECK(splice.getLogicalSpelling().getLength() == 0);
        auto eof = input.lexer.readLexeme();
        SLANG_CHECK(eof.is<RawEndOfFile>());
        SLANG_CHECK(eof.getLoc() == input.beginLoc + 4);
        SLANG_CHECK(input.sink.getErrorCount() == 0);
    }
}

/// Checks that incomplete UTF-8 is diagnosed and consumed only through the bounded view's end.
SLANG_UNIT_TEST(lexerTruncatedUtf8UsesBufferEnd)
{
    // E2 82 begins a three-byte UTF-8 sequence. The third byte lies outside the view,
    // so recognition must diagnose the incomplete sequence and stop at the view's end.
    const char bytes[] = {char(0xE2), char(0x82), char(0xAC)};
    LexemeTestInput input(UnownedStringSlice(bytes, size_t(2)));
    auto invalidSequence = input.lexer.readLexeme();
    SLANG_CHECK(invalidSequence.getPhysicalSpelling() == UnownedStringSlice(bytes, size_t(2)));
    SLANG_CHECK(input.sink.getErrorCount() == 1);
    auto eof = input.lexer.readLexeme();
    SLANG_CHECK(eof.is<RawEndOfFile>());
    SLANG_CHECK(eof.getLoc() == input.beginLoc + 2);
}

/// Checks that invalid UTF-8 recovery preserves a following ASCII identifier.
SLANG_UNIT_TEST(lexerInvalidUtf8PreservesFollowingAscii)
{
    // The ASCII `b` cannot continue E2 82. After diagnosing those two bytes, the
    // lexer must leave `b` for the next identifier rather than consume it as UTF-8.
    const char bytes[] = {char(0xE2), char(0x82), 'b'};
    LexemeTestInput input(UnownedStringSlice(bytes, sizeof(bytes)));
    auto invalidSequence = input.lexer.readLexeme();
    SLANG_CHECK(invalidSequence.getPhysicalSpelling() == UnownedStringSlice(bytes, size_t(2)));
    SLANG_CHECK(input.sink.getErrorCount() == 1);
    auto identifier = input.lexer.readLexeme();
    SLANG_CHECK(identifier.is<RawIdentifier>());
    SLANG_CHECK(identifier.getLogicalSpelling() == UnownedStringSlice("b"));
    SLANG_CHECK(identifier.getLoc() == input.beginLoc + 2);
    SLANG_CHECK(input.lexer.readLexeme().is<RawEndOfFile>());
    SLANG_CHECK(input.sink.getErrorCount() == 1);
}

/// Checks that out-of-range UTF-8 cannot become EOF and preserves following input.
SLANG_UNIT_TEST(lexerInvalidUnicodeCannotBecomeEndOfFile)
{
    // The extended seven-byte encoding denotes FFFFFFFF, which becomes -1 if cast
    // to a signed 32-bit integer. The lexer must reject that code point before it
    // can be confused with EOF, preserving both the invalid bytes and the next token.
    // `_readCodePoint()` recovers from an invalid encoding by returning a space to the scanner.
    // We therefore expect `WhiteSpaceTrivia` that preserves all seven physical bytes.
    const char bytes[] =
        {char(0xFE), char(0x83), char(0xBF), char(0xBF), char(0xBF), char(0xBF), char(0xBF), 'x'};
    LexemeTestInput input(UnownedStringSlice(bytes, sizeof(bytes)));
    auto invalidSequence = input.lexer.readLexeme();
    SLANG_CHECK(invalidSequence.is<WhiteSpaceTrivia>());
    SLANG_CHECK(invalidSequence.getPhysicalSpelling() == UnownedStringSlice(bytes, size_t(7)));
    SLANG_CHECK(input.sink.getErrorCount() == 1);
    auto identifier = input.lexer.readLexeme();
    SLANG_CHECK(identifier.is<RawIdentifier>());
    SLANG_CHECK(identifier.getLogicalSpelling() == UnownedStringSlice("x"));
    auto eof = input.lexer.readLexeme();
    SLANG_CHECK(eof.is<RawEndOfFile>());
    SLANG_CHECK(eof.getLoc() == input.beginLoc + sizeof(bytes));
    SLANG_CHECK(input.sink.getErrorCount() == 1);

    StringBuilder reconstructed;
    reconstructed << invalidSequence.getPhysicalSpelling() << identifier.getPhysicalSpelling()
                  << eof.getPhysicalSpelling();
    SLANG_CHECK(reconstructed.getUnownedSlice() == UnownedStringSlice(bytes, sizeof(bytes)));
}

/// Checks that the lexer accepts U+10FFFF, the largest Unicode scalar value, in an identifier.
SLANG_UNIT_TEST(lexerAcceptsMaximumUnicodeScalar)
{
    const char bytes[] = {char(0xF4), char(0x8F), char(0xBF), char(0xBF)};
    LexemeTestInput input(UnownedStringSlice(bytes, sizeof(bytes)));
    auto identifier = input.lexer.readLexeme();
    SLANG_CHECK(identifier.is<RawIdentifier>());
    SLANG_CHECK(identifier.getPhysicalSpelling() == UnownedStringSlice(bytes, sizeof(bytes)));
    SLANG_CHECK(input.lexer.readLexeme().is<RawEndOfFile>());
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

/// Checks that surrogate encodings are diagnosed while preserving the following identifier.
SLANG_UNIT_TEST(lexerRejectsUnicodeSurrogate)
{
    // ED A0 80 encodes the surrogate U+D800, which is not a Unicode scalar value.
    // Recovery must consume those three bytes and leave the ASCII identifier intact.
    // `_readCodePoint()` returns a recovery space, so we expect `WhiteSpaceTrivia`
    // that preserves the three physical bytes of the invalid encoding.
    const char bytes[] = {char(0xED), char(0xA0), char(0x80), 'x'};
    LexemeTestInput input(UnownedStringSlice(bytes, sizeof(bytes)));
    auto invalidSequence = input.lexer.readLexeme();
    SLANG_CHECK(invalidSequence.is<WhiteSpaceTrivia>());
    SLANG_CHECK(invalidSequence.getPhysicalSpelling() == UnownedStringSlice(bytes, size_t(3)));
    SLANG_CHECK(input.sink.getErrorCount() == 1);
    auto identifier = input.lexer.readLexeme();
    SLANG_CHECK(identifier.is<RawIdentifier>());
    SLANG_CHECK(identifier.getLogicalSpelling() == UnownedStringSlice("x"));
    SLANG_CHECK(input.lexer.readLexeme().is<RawEndOfFile>());
    SLANG_CHECK(input.sink.getErrorCount() == 1);
}

/// Checks that a spliced identifier keeps both original bytes and its logical name.
SLANG_UNIT_TEST(lexerRetainsPhysicalAndLogicalSpellings)
{
    // Identifier lookup uses the spelling after line splicing, while reconstruction
    // uses the original CRLF continuation. Neither operation may overwrite the other.
    LexemeTestInput input("alpha\\\r\nbeta");
    auto identifier = input.lexer.readLexeme();
    SLANG_CHECK(identifier.is<RawIdentifier>());
    SLANG_CHECK(identifier.getPhysicalSpelling() == UnownedStringSlice("alpha\\\r\nbeta"));
    SLANG_CHECK(identifier.getLogicalSpelling() == UnownedStringSlice("alphabeta"));
    SLANG_CHECK(identifier.getFlags() == (TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace));
    SLANG_CHECK(identifier.getName() == input.namePool.getName(String("alphabeta")));
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

/// Checks that the reported delimiter matches attached trivia and is cleared by every read.
SLANG_UNIT_TEST(lexerReportsItsTrailingNewLine)
{
    // We compare the recognition result with the existing located-trivia API.
    // Comments and splices can contain physical newlines without ending a directive;
    // blank lines after the first delimiter instead lead the next token or EOF.
    const char* inputs[] = {
        "a /*embedded\nline*/ \\\r\n \r\n\n b c",
        "a\r\nb\n\rc\rd\n",
        "a\\\nb",
        "a \\\nb",
        "/*only\ntrivia*/\n",
        "a\n",
        "",
    };
    const bool baseLocationCases[] = {true, false};
    for (bool hasKnownBaseLocation : baseLocationCases)
    {
        for (const auto text : inputs)
        {
            LexemeTestInput input(text);
            // An unknown raw-token location also makes its attached trivia unlocated.
            // Clearing the base exercises that case when the raw token begins at byte zero.
            if (!hasKnownBaseLocation)
                input.lexer.m_startLoc = SourceLoc();

            // A nonempty initial value detects stale results even for an empty input.
            auto reported =
                input.tokenPool->createLexeme(LexemeTypeTag::NewLine, toSlice("\n"), toSlice("\n"))
                    .as<NewLineTrivia>();
            for (;;)
            {
                const auto token = input.lexer.readToken(&reported);
                NewLineTrivia attached;
                for (auto trivia : token.getTrailingTrivia())
                {
                    if (auto newLine = trivia.as<NewLineTrivia>())
                        attached = newLine;
                }
                SLANG_CHECK(reported.getObject() == attached.getObject());
                SLANG_CHECK(reported.getLoc() == attached.getLoc());
                SLANG_CHECK(reported.getFlags() == 0);
                if (token.is<EndOfFileToken>())
                    break;
            }
        }
    }
}

/// Checks that attached trivia partitions the input and preserves block comments across newlines.
SLANG_UNIT_TEST(lexerTokenTriviaPartitionsPhysicalInput)
{
    // The newline inside a block comment does not split that lexeme. Trailing trivia
    // ends at the first explicit newline; blank lines and indentation then lead the
    // following token. EOF owns the final indentation after the last newline.
    const char* text = "  first /*one\n two*/ \r\n\n\t//lead\n second //tail\r\n  ";
    LexemeTestInput input(text);
    auto first = input.lexer.readToken();
    auto second = input.lexer.readToken();
    auto eof = input.lexer.readToken();
    SLANG_CHECK(first.getLogicalSpelling() == UnownedStringSlice("first"));
    SLANG_CHECK(first.getLoc() == input.beginLoc + 2);
    SLANG_CHECK(first.getLeadingTrivia().getCount() == 1);
    SLANG_CHECK_ABORT(first.getTrailingTrivia().getCount() == 4);
    SLANG_CHECK(first.getTrailingTrivia()[1].is<BlockCommentTrivia>());
    SLANG_CHECK(first.getTrailingTrivia()[3].is<NewLineTrivia>());
    SLANG_CHECK(second.getLeadingTrivia().getCount() == 5);
    SLANG_CHECK(second.getTrailingTrivia().getCount() == 3);
    SLANG_CHECK(eof.is<EndOfFileToken>());
    SLANG_CHECK(eof.getLoc() == input.beginLoc + UnownedStringSlice(text).getLength());
    SLANG_CHECK(eof.getLeadingTrivia().getCount() == 1);

    // Joining every cluster in order must reproduce the input exactly, with no
    // duplicated comment or newline at the division between adjacent tokens.
    StringBuilder reconstructed;
    for (auto token : {first, second, eof})
    {
        for (auto trivia : token.getLeadingTrivia())
            reconstructed << trivia.getPhysicalSpelling();
        reconstructed << token.getPhysicalSpelling();
        for (auto trivia : token.getTrailingTrivia())
            reconstructed << trivia.getPhysicalSpelling();
    }
    SLANG_CHECK(reconstructed.getUnownedSlice() == UnownedStringSlice(text));
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

/// Checks that a trivia-only input is retained entirely as a real EOF token's leading trivia.
SLANG_UNIT_TEST(lexerTriviaOnlyFileUsesEndOfFileToken)
{
    const char* text = " //only\n\t/*last*/";
    LexemeTestInput input(text);
    auto eof = input.lexer.readToken();
    SLANG_CHECK(eof.is<EndOfFileToken>());
    SLANG_CHECK(eof.getLeadingTrivia().getCount() == 5);
    SLANG_CHECK(eof.getTrailingTrivia().getCount() == 0);
    StringBuilder reconstructed;
    for (auto trivia : eof.getLeadingTrivia())
        reconstructed << trivia.getPhysicalSpelling();
    SLANG_CHECK(reconstructed.getUnownedSlice() == UnownedStringSlice(text));
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

/// Checks that repeated reader EOF results retain the stored occurrence and its trivia.
SLANG_UNIT_TEST(tokenReaderPreservesStoredEndOfFileTrivia)
{
    // The first newline trails `a`; the remaining newline and comment lead EOF.
    LexemeTestInput input("a\n\n//last");
    auto tokens = input.lexer.readAllTokens();
    const auto storedEOF = *tokens.end();
    SLANG_CHECK_ABORT(storedEOF.is<EndOfFileToken>());
    SLANG_CHECK_ABORT(storedEOF.getLeadingTrivia().getCount() == 2);
    SLANG_CHECK(storedEOF.getLeadingTrivia()[0].getPhysicalSpelling() == UnownedStringSlice("\n"));
    SLANG_CHECK(
        storedEOF.getLeadingTrivia()[1].getPhysicalSpelling() == UnownedStringSlice("//last"));
    TokenReader reader(tokens);
    SLANG_CHECK(
        reader.advanceIf<IdentifierToken>().getLogicalSpelling() == UnownedStringSlice("a"));

    // Reading EOF repeatedly is stable: both the interned cluster and its occurrence
    // state remain the same, including the positions of its attached trivia.
    for (int read = 0; read < 3; ++read)
    {
        auto eof = reader.readToken();
        SLANG_CHECK(eof.getObject() == storedEOF.getObject());
        SLANG_CHECK(eof.getLoc() == storedEOF.getLoc());
        SLANG_CHECK(eof.getFlags() == storedEOF.getFlags());
        SLANG_CHECK_ABORT(eof.getLeadingTrivia().getCount() == 2);
        SLANG_CHECK(eof.getLeadingTrivia()[1].getLoc() == storedEOF.getLeadingTrivia()[1].getLoc());
    }
}

/// Checks that a bounded reader repeats its supplied EOF occurrence at the range end.
SLANG_UNIT_TEST(tokenReaderBoundedRangeUsesExplicitEndOfFile)
{
    // The range excludes `two`. Its supplied EOF has a different location and flags
    // from the file's EOF, so the reader cannot derive its endpoint from either token.
    LexemeTestInput input("one two");
    auto tokens = input.lexer.readAllTokens();
    RefPtr<TokenPool> pool = new TokenPool();
    auto endpoint = pool->createEndOfFile(SourceLoc::fromRaw(150), TokenFlag::AfterWhitespace);
    TokenReader reader(tokens.begin(), tokens.begin() + 1, endpoint);
    SLANG_CHECK(
        reader.advanceIf<IdentifierToken>().getLogicalSpelling() == UnownedStringSlice("one"));
    SLANG_CHECK(reader.isAtEnd());
    for (int read = 0; read < 3; ++read)
    {
        auto eof = reader.readToken();
        SLANG_CHECK(eof.getObject() == endpoint.getObject());
        SLANG_CHECK(eof.getLoc() == endpoint.getLoc());
        SLANG_CHECK(eof.getFlags() == endpoint.getFlags());
    }
}

/// Checks that an empty reader needs no backing array and repeats its supplied EOF occurrence.
SLANG_UNIT_TEST(tokenReaderEmptyRangeUsesExplicitEndOfFile)
{
    RefPtr<TokenPool> pool = new TokenPool();
    auto endpoint = pool->createEndOfFile(SourceLoc::fromRaw(150));
    TokenReader reader(nullptr, nullptr, endpoint);
    SLANG_CHECK(reader.isAtEnd());
    SLANG_CHECK(reader.getCount() == 0);
    SLANG_CHECK(reader.peekToken().getObject() == endpoint.getObject());
    SLANG_CHECK(reader.readToken().getLoc() == endpoint.getLoc());
    SLANG_CHECK(reader.readToken().getObject() == endpoint.getObject());
}

/// Checks that reader cursors restore both the original `>>` and its unconsumed second closer.
SLANG_UNIT_TEST(tokenReaderRestoresPartiallyConsumedRightShift)
{
    // After consuming the first closer of `>>`, the parser replaces lookahead with
    // the second closer. A saved cursor must retain that replacement as well as the
    // array position, so speculative parsing can restore either interpretation.
    LexemeTestInput input(">\\\n> //tail\nnext");
    auto tokens = input.lexer.readAllTokens();
    TokenReader reader(tokens);
    auto beforeSplit = reader.getCursor();
    auto original = reader.peekToken();
    SLANG_CHECK_ABORT(original.is<OpRshToken>());
    auto pool = original.getPool();
    auto secondRaw = pool->createSecondGreaterToken(original.getRawToken().as<RawOpRsh>());
    auto second = pool->createToken(secondRaw, {}, original.getTrailingTrivia().getObjects());
    reader.replaceLookaheadToken(second);
    auto afterSplit = reader.getCursor();
    SLANG_CHECK(!reader.isAtCursor(beforeSplit));
    SLANG_CHECK(reader.readToken().getObject() == second.getObject());
    SLANG_CHECK(reader.is<IdentifierToken>());

    reader.setCursor(afterSplit);
    auto restored = reader.readToken();
    SLANG_CHECK(restored.is<OpGreaterToken>());
    SLANG_CHECK(restored.getObject() == second.getObject());
    SLANG_CHECK(restored.getLoc() == input.beginLoc + 3);
    SLANG_CHECK(restored.getFlags() == 0);
    SLANG_CHECK_ABORT(restored.getTrailingTrivia().getCount() == 3);
    SLANG_CHECK(
        restored.getTrailingTrivia()[1].getPhysicalSpelling() == UnownedStringSlice("//tail"));
    SLANG_CHECK(
        reader.advanceIf<IdentifierToken>().getLogicalSpelling() == UnownedStringSlice("next"));

    reader.setCursor(beforeSplit);
    SLANG_CHECK(reader.peekToken().getObject() == original.getObject());
    SLANG_CHECK(reader.readToken().is<OpRshToken>());
    SLANG_CHECK(reader.is<IdentifierToken>());
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

/// Checks that reading trailing trivia defers recognition and diagnostics for the next raw token.
SLANG_UNIT_TEST(lexerTrailingTriviaDoesNotReadNextRawToken)
{
    // The next string literal is malformed. Reading the identifier may consume its
    // trailing space, but must leave both recognition and diagnostics for the string
    // until the caller asks for the next token.
    LexemeTestInput input("name \"unterminated\n");
    auto identifier = input.lexer.readToken();
    SLANG_CHECK(identifier.is<IdentifierToken>());
    SLANG_CHECK(input.sink.getErrorCount() == 0);
    input.lexer.readToken();
    SLANG_CHECK(input.sink.getErrorCount() > 0);
}

/// Checks that token reads stop at the first trailing newline before a diagnostic-policy change.
SLANG_UNIT_TEST(lexerTrailingTriviaStopsBeforeFollowingLine)
{
    // A preprocessor can change diagnostic policy after reading the complete directive
    // line. Even indentation and a comment on the next line must remain unlexed until
    // that change; the malformed string must then obey the new suppression policy.
    LexemeTestInput input("directive\n /*next*/ \"unterminated\n");
    auto directive = input.lexer.readToken();
    SLANG_CHECK(directive.getTrailingTrivia().getCount() == 1);
    SLANG_CHECK(
        UnownedStringSlice(input.lexer.m_begin, input.lexer.m_cursor) ==
        UnownedStringSlice("directive\n"));
    SLANG_CHECK(input.sink.getErrorCount() == 0);
    input.lexer.m_lexerFlags |= kLexerFlag_SuppressDiagnostics;
    SLANG_CHECK(input.lexer.readToken().is<StringLiteralToken>());
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

/// Checks that header-name mode can be selected after `include` even with no intervening space.
SLANG_UNIT_TEST(lexerAngleBracketHeaderNameModePrecedesAdjacentOperand)
{
    // There is no space between "include" and its operand. Switching mode after
    // receiving the identifier must still recognize the complete angle-bracket name,
    // including characters that would otherwise start a C-style comment.
    LexemeTestInput input("#include<one/*name*/two.h>\nnext");
    SLANG_CHECK(input.lexer.readToken().is<PoundToken>());
    SLANG_CHECK(input.lexer.readToken().getLogicalSpelling() == UnownedStringSlice("include"));
    input.lexer.setMode(LexerMode::ExpectingAngleBracketHeaderName);
    auto header = input.lexer.readToken();
    SLANG_CHECK(header.is<AngleBracketHeaderNameToken>());
    SLANG_CHECK(header.getPhysicalSpelling() == UnownedStringSlice("<one/*name*/two.h>"));
    input.lexer.setMode(LexerMode::Normal);
    SLANG_CHECK(input.lexer.readToken().getLogicalSpelling() == UnownedStringSlice("next"));
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

/// Checks raw and ordinary string decoding, including incomplete and unterminated delimiters.
SLANG_UNIT_TEST(lexerStringLiteralValuesDistinguishRawAndEscapedText)
{
    // Ordinary strings decode escapes, while a raw string preserves the same backslash
    // bytes literally. A named delimiter must be removed from both ends of a raw string.
    LexemeTestInput ordinary("\"a\\n\\u03BB\"");
    auto ordinaryToken = ordinary.lexer.readToken().as<StringLiteralToken>();
    SLANG_CHECK_ABORT(bool(ordinaryToken));
    SLANG_CHECK(getStringLiteralTokenValue(ordinaryToken, &ordinary.sink) == "a\n\xCE\xBB");
    SLANG_CHECK(ordinary.sink.getErrorCount() == 0);

    LexemeTestInput raw("R\"tag(a\\n\\u03BB)tag\"");
    auto rawToken = raw.lexer.readToken().getRawToken().as<RawStringLiteral>();
    SLANG_CHECK_ABORT(bool(rawToken));
    SLANG_CHECK(getStringLiteralTokenValue(rawToken, &raw.sink) == "a\\n\\u03BB");
    SLANG_CHECK(raw.sink.getErrorCount() == 0);

    // The lexer diagnoses incomplete raw delimiters. Decoding must not form a body slice
    // outside the available token spelling or mistake a mismatched delimiter for a close.
    const char* incompleteRawStrings[] = {"R\"tag", "R\"tag(body", "R\"tag(body)other\""};
    for (auto text : incompleteRawStrings)
    {
        LexemeTestInput input(text);
        auto token = input.lexer.readToken().as<StringLiteralToken>();
        SLANG_CHECK_ABORT(bool(token));
        SLANG_CHECK(getStringLiteralTokenValue(token, &input.sink).getLength() == 0);
        SLANG_CHECK(input.sink.getErrorCount() > 0);
    }

    // An ordinary unterminated string still yields its decoded prefix. This buffer has
    // no terminator, so even the final escape must stay within the supplied byte range.
    const char bytes[] = {'"', 'a', '\\', 'n'};
    LexemeTestInput incomplete(UnownedStringSlice(bytes, sizeof(bytes)));
    auto token = incomplete.lexer.readToken().as<StringLiteralToken>();
    SLANG_CHECK_ABORT(bool(token));
    SLANG_CHECK(getStringLiteralTokenValue(token, &incomplete.sink) == "a\n");
    SLANG_CHECK(incomplete.sink.getErrorCount() > 0);
}

/// Checks complete-character decoding for escaped and UTF-8 literals and rejects invalid bodies.
SLANG_UNIT_TEST(lexerCharacterLiteralValuesRequireOneCompleteCharacter)
{
    // A Unicode escape and the corresponding UTF-8 bytes denote the same code point.
    // Both must consume the complete quoted body, rather than only its first byte.
    const char* validCharacters[] = {"'\\u03BB'", "'\xCE\xBB'"};
    for (auto text : validCharacters)
    {
        LexemeTestInput input(text);
        auto token = input.lexer.readToken().as<CharLiteralToken>();
        SLANG_CHECK_ABORT(bool(token));
        SLANG_CHECK(getCharLiteralValue(token, &input.sink) == 0x03BB);
        SLANG_CHECK(input.sink.getErrorCount() == 0);
    }

    // Empty bodies, multiple characters, and missing closing quotes cannot yield a
    // character value. In particular, `'ab` must not lose its final byte and decode as `a`.
    const char* invalidCharacters[] = {"''", "'ab'", "'ab"};
    for (auto text : invalidCharacters)
    {
        LexemeTestInput input(text);
        auto token = input.lexer.readToken().getRawToken().as<RawCharLiteral>();
        SLANG_CHECK_ABORT(bool(token));
        SLANG_CHECK(getCharLiteralValue(token, &input.sink) == -1);
        SLANG_CHECK(input.sink.getErrorCount() > 0);
    }
}

/// Checks that concrete descriptors and abstract ranges agree with the lexical categories.
SLANG_UNIT_TEST(lexemeTypeRangesClassifyConcreteTags)
{
    // Every concrete tag indexes one leaf descriptor. Abstract ranges must agree
    // with the grammar categories independently of the numerical DFS ordering.
    static_assert(sizeof(LexemeTypeTag) == 1);
    for (unsigned i = 0; i < unsigned(LexemeTypeTag::Count); ++i)
    {
        auto tag = LexemeTypeTag(i);
        const auto& leaf = getLexemeType(tag);
        SLANG_CHECK(leaf.tagCount == 1);
        SLANG_CHECK(leaf.firstTag == tag);
        SLANG_CHECK(leaf.contains(tag));
        SLANG_CHECK(!leaf.contains(leaf.getEndTag()));
        SLANG_CHECK(
            UnownedStringSlice(TokenTypeToString(tag)) == UnownedStringSlice(leaf.description));
        SLANG_CHECK(Lexeme::type.contains(tag));
        bool isTrivia = false;
        switch (tag)
        {
        case LexemeTypeTag::WhiteSpace:
        case LexemeTypeTag::NewLine:
        case LexemeTypeTag::EscapedNewLine:
        case LexemeTypeTag::LineComment:
        case LexemeTypeTag::BlockComment:
            isTrivia = true;
            break;
        default:
            break;
        }
        SLANG_CHECK(Trivia::type.contains(tag) == isTrivia);
        SLANG_CHECK(RawToken::type.contains(tag) == !isTrivia);
        bool isNumeric =
            tag == LexemeTypeTag::IntegerLiteral || tag == LexemeTypeTag::FloatingPointLiteral;
        SLANG_CHECK(RawNumericLiteral::type.contains(tag) == isNumeric);
    }
    SLANG_CHECK(!Lexeme::type.contains(LexemeTypeTag::Count));
    SLANG_CHECK(!Trivia::type.contains(LexemeTypeTag(255)));
    SLANG_CHECK(!RawToken::type.contains(LexemeTypeTag::Count));
    SLANG_CHECK(RawPunctuation::type.contains(RawOpAdd::tag));
    SLANG_CHECK(!RawPunctuation::type.contains(RawIdentifier::tag));
    SLANG_CHECK(RawLiteral::type.contains(RawStringLiteral::tag));
    SLANG_CHECK(!RawLiteral::type.contains(RawIdentifier::tag));
}

/// Checks that names and both spellings remain part of identity across repeated requests.
SLANG_UNIT_TEST(lexemePoolDistinguishesNamesAndSpellings)
{
    // Equal text from separate name pools still gives distinct names. Alternating
    // those names and spellings must not replace the object for an earlier request.
    RefPtr<TokenPool> pool = new TokenPool();
    NamePool firstNames;
    NamePool secondNames;
    const auto firstName = firstNames.getName(toSlice("same"));
    const auto secondName = secondNames.getName(toSlice("same"));
    SLANG_CHECK(firstName != secondName);

    const auto initial = pool->createLexeme(
        LexemeTypeTag::Identifier,
        toSlice("same"),
        toSlice("same"),
        SourceLoc::fromRaw(100),
        TokenFlag::AtStartOfLine,
        firstName);
    const auto otherName = pool->createLexeme(
        LexemeTypeTag::Identifier,
        toSlice("same"),
        toSlice("same"),
        {},
        0,
        secondName);
    const auto noName =
        pool->createLexeme(LexemeTypeTag::Identifier, toSlice("same"), toSlice("same"));
    const auto spliced = pool->createLexeme(
        LexemeTypeTag::Identifier,
        toSlice("sa\\\nme"),
        toSlice("same"),
        {},
        0,
        firstName);

    // The factory also accepts explicitly supplied logical spellings for synthetic
    // lexemes. Their identity cannot be inferred from the physical spelling alone.
    const auto otherLogical =
        pool->createLexeme(LexemeTypeTag::Identifier, toSlice("same"), toSlice("different"));
    SLANG_CHECK(initial.getObject() != otherName.getObject());
    SLANG_CHECK(initial.getObject() != noName.getObject());
    SLANG_CHECK(initial.getObject() != spliced.getObject());
    SLANG_CHECK(noName.getObject() != otherLogical.getObject());

    // A later request must recover its original object after those intervening
    // states, while the returned occurrence keeps its new location and flags.
    const auto repeated = pool->createLexeme(
        LexemeTypeTag::Identifier,
        toSlice("same"),
        toSlice("same"),
        SourceLoc::fromRaw(200),
        TokenFlag::AfterWhitespace,
        firstName);
    SLANG_CHECK(repeated.getObject() == initial.getObject());
    SLANG_CHECK(repeated.getLoc() == SourceLoc::fromRaw(200));
    SLANG_CHECK(repeated.getFlags() == TokenFlag::AfterWhitespace);
    const auto adjacentRepeat = pool->createLexeme(
        LexemeTypeTag::Identifier,
        toSlice("same"),
        toSlice("same"),
        {},
        0,
        firstName);
    SLANG_CHECK(adjacentRepeat.getObject() == initial.getObject());
    SLANG_CHECK(adjacentRepeat.getLoc() == SourceLoc());
    SLANG_CHECK(adjacentRepeat.getFlags() == 0);
}

/// Checks that each newline spelling keeps its own canonical object across alternating requests.
SLANG_UNIT_TEST(lexemePoolDistinguishesNewLineSpellings)
{
    // A file may mix line endings. Sharing one lexeme type does not make these
    // byte sequences interchangeable when reconstructing the physical input.
    RefPtr<TokenPool> pool = new TokenPool();
    const char* spellings[] = {"\n", "\r", "\r\n", "\n\r"};
    const LexemeObj* objects[SLANG_COUNT_OF(spellings)] = {};
    for (Index i = 0; i < SLANG_COUNT_OF(spellings); ++i)
    {
        const auto spelling = UnownedStringSlice(spellings[i]);
        objects[i] = pool->createLexeme(LexemeTypeTag::NewLine, spelling, spelling).getObject();
        for (Index earlier = 0; earlier < i; ++earlier)
            SLANG_CHECK(objects[i] != objects[earlier]);
    }
    for (Index i = 0; i < SLANG_COUNT_OF(spellings); ++i)
    {
        const auto spelling = UnownedStringSlice(spellings[i]);
        const auto repeated = pool->createLexeme(LexemeTypeTag::NewLine, spelling, spelling);
        SLANG_CHECK(repeated.getObject() == objects[i]);
        SLANG_CHECK(repeated.getPhysicalSpelling() == spelling);
        SLANG_CHECK(repeated.getLogicalSpelling() == spelling);
    }
}

/// Checks that lexing and generic creation share fixed spellings in either creation order.
SLANG_UNIT_TEST(lexerFixedSpellingsShareGenericLexemeObjects)
{
    // Generic creation can supply a name or a synthetic spelling even for a
    // punctuation tag. Lexing `++` must select the canonical unnamed `++` state,
    // while `+` followed by a splice and another `+` keeps a distinct physical spelling.
    LexemeTestInput input("++ +\\\n+ ... .. <<=");
    const auto namedSpelling = input.namePool.getName(toSlice("++"));
    const auto named = input.tokenPool->createLexeme(
        LexemeTypeTag::OpInc,
        toSlice("++"),
        toSlice("++"),
        {},
        0,
        namedSpelling);
    const auto synthetic = input.tokenPool->createLexeme(
        LexemeTypeTag::OpInc,
        toSlice("synthetic"),
        toSlice("synthetic"));
    const auto generic =
        input.tokenPool->createLexeme(LexemeTypeTag::OpInc, toSlice("++"), toSlice("++"));
    const auto ordinary = input.lexer.readToken();
    const auto spliced = input.lexer.readToken();
    SLANG_CHECK(ordinary.is<OpIncToken>());
    SLANG_CHECK(spliced.is<OpIncToken>());
    SLANG_CHECK(ordinary.getRawToken().getObject() == generic.getObject());
    SLANG_CHECK(ordinary.getRawToken().getObject() != named.getObject());
    SLANG_CHECK(ordinary.getRawToken().getObject() != synthetic.getObject());
    SLANG_CHECK(spliced.getRawToken().getObject() != generic.getObject());
    SLANG_CHECK(spliced.getPhysicalSpelling() == toSlice("+\\\n+"));
    SLANG_CHECK(spliced.getLogicalSpelling() == toSlice("++"));

    // These other multi-byte spellings are lexed before any generic request.
    // Generic creation must find those same objects rather than create a second identity.
    const auto ellipsis = input.lexer.readToken();
    const auto dotDot = input.lexer.readToken();
    const auto shiftAssign = input.lexer.readToken();
    SLANG_CHECK(ellipsis.is<EllipsisToken>());
    SLANG_CHECK(dotDot.is<DotDotToken>());
    SLANG_CHECK(shiftAssign.is<OpShlAssignToken>());
    SLANG_CHECK(
        input.tokenPool->createLexeme(LexemeTypeTag::Ellipsis, toSlice("..."), toSlice("..."))
            .getObject() == ellipsis.getRawToken().getObject());
    SLANG_CHECK(
        input.tokenPool->createLexeme(LexemeTypeTag::DotDot, toSlice(".."), toSlice(".."))
            .getObject() == dotDot.getRawToken().getObject());
    SLANG_CHECK(
        input.tokenPool->createLexeme(LexemeTypeTag::OpShlAssign, toSlice("<<="), toSlice("<<="))
            .getObject() == shiftAssign.getRawToken().getObject());
}

/// Checks that completion requests retain their names despite having a fixed spelling.
SLANG_UNIT_TEST(lexerCompletionRequestsPreserveNames)
{
    // The lexer gives `#?` a name. An unnamed request with the same tag and bytes
    // must remain a separate state, even when it was the most recently created one.
    LexemeTestInput input("#? #?");
    const auto name = input.namePool.getName(toSlice("#?"));
    const auto named = input.tokenPool->createLexeme(
        LexemeTypeTag::CompletionRequest,
        toSlice("#?"),
        toSlice("#?"),
        {},
        0,
        name);
    const auto unnamed = input.tokenPool->createLexeme(
        LexemeTypeTag::CompletionRequest,
        toSlice("#?"),
        toSlice("#?"));
    const auto first = input.lexer.readToken();
    const auto second = input.lexer.readToken();
    SLANG_CHECK(first.is<CompletionRequestToken>());
    SLANG_CHECK(first.getRawToken().getObject() == named.getObject());
    SLANG_CHECK(second.getRawToken().getObject() == named.getObject());
    SLANG_CHECK(first.getRawToken().getObject() != unnamed.getObject());
    SLANG_CHECK(first.getName() == name);
}

/// Checks that interning preserves raw-token and ordered-trivia identity after other requests.
SLANG_UNIT_TEST(tokenPoolReusePreservesRawTokenAndTriviaIdentity)
{
    // The same trivia objects can form different clusters by changing their order
    // or their attachment to the raw token. A different raw spelling also changes
    // cluster identity, even when its tag and the attached trivia remain the same.
    RefPtr<TokenPool> pool = new TokenPool();
    const auto raw =
        pool->createLexeme(LexemeTypeTag::OpAdd, toSlice("+"), toSlice("+")).as<RawToken>();
    const auto otherRaw =
        pool->createLexeme(LexemeTypeTag::OpAdd, toSlice("other"), toSlice("other")).as<RawToken>();
    const auto space = pool->createLexeme(LexemeTypeTag::WhiteSpace, toSlice(" "), toSlice(" "));
    const auto comment =
        pool->createLexeme(LexemeTypeTag::BlockComment, toSlice("/**/"), toSlice("/**/"));
    const auto newline = pool->createLexeme(LexemeTypeTag::NewLine, toSlice("\n"), toSlice("\n"));
    const LexemeObj* leading[] = {space.getObject(), comment.getObject()};
    const LexemeObj* reversed[] = {comment.getObject(), space.getObject()};
    const LexemeObj* trailing[] = {newline.getObject()};
    const LexemeObj* shiftedTrailing[] = {comment.getObject(), newline.getObject()};
    const auto initial =
        pool->createToken(raw, makeConstArrayView(leading), makeConstArrayView(trailing));
    const auto changedOrder =
        pool->createToken(raw, makeConstArrayView(reversed), makeConstArrayView(trailing));
    const auto changedDivision =
        pool->createToken(raw, makeConstArrayView(leading, 1), makeConstArrayView(shiftedTrailing));
    const auto changedRaw =
        pool->createToken(otherRaw, makeConstArrayView(leading), makeConstArrayView(trailing));
    SLANG_CHECK(initial.getObject() != changedOrder.getObject());
    SLANG_CHECK(initial.getObject() != changedDivision.getObject());
    SLANG_CHECK(initial.getObject() != changedRaw.getObject());

    // Recovering the original cluster must use the new occurrence's location and
    // flags. A following request with default values must receive those defaults.
    const auto relocatedLexeme = pool->createLexeme(
        LexemeTypeTag::OpAdd,
        toSlice("+"),
        toSlice("+"),
        SourceLoc::fromRaw(100),
        TokenFlag::AfterWhitespace);
    const auto repeated = pool->createToken(
        relocatedLexeme.as<RawToken>(),
        makeConstArrayView(leading),
        makeConstArrayView(trailing));
    SLANG_CHECK(repeated.getObject() == initial.getObject());
    SLANG_CHECK(repeated.getLoc() == SourceLoc::fromRaw(100));
    SLANG_CHECK(repeated.getFlags() == TokenFlag::AfterWhitespace);
    const auto adjacentRepeat =
        pool->createToken(raw, makeConstArrayView(leading), makeConstArrayView(trailing));
    SLANG_CHECK(adjacentRepeat.getObject() == initial.getObject());
    SLANG_CHECK(adjacentRepeat.getLoc() == SourceLoc());
    SLANG_CHECK(adjacentRepeat.getFlags() == 0);
}

/// Checks that each token pool maintains independent lexeme and token identities.
SLANG_UNIT_TEST(tokenPoolsKeepCanonicalObjectsSeparate)
{
    // Equal requests in two live pools must produce different objects. Repeating
    // each request must still recover the object owned by its selected pool.
    RefPtr<TokenPool> first = new TokenPool();
    RefPtr<TokenPool> second = new TokenPool();
    const auto firstToken = first->createToken(TokenType::OpInc, toSlice("++"));
    const auto secondToken = second->createToken(TokenType::OpInc, toSlice("++"));
    SLANG_CHECK(firstToken.getObject() != secondToken.getObject());
    SLANG_CHECK(firstToken.getRawToken().getObject() != secondToken.getRawToken().getObject());
    SLANG_CHECK(firstToken.getPool() == first.get());
    SLANG_CHECK(secondToken.getPool() == second.get());
    const auto repeatedFirstToken = first->createToken(TokenType::OpInc, toSlice("++"));
    const auto repeatedSecondToken = second->createToken(TokenType::OpInc, toSlice("++"));
    SLANG_CHECK(repeatedFirstToken.getObject() == firstToken.getObject());
    SLANG_CHECK(repeatedSecondToken.getObject() == secondToken.getObject());
}

/// Checks identifier spelling and context across ASCII, Unicode, and internal splice transitions.
SLANG_UNIT_TEST(lexerIdentifierContinuesAfterUnicodeAndSplices)
{
    /// A physical identifier spelling and its expected logical spelling.
    struct IdentifierSpellingCase
    {
        const char* physicalSpelling;
        const char* logicalSpelling;
    };
    const IdentifierSpellingCase cases[] = {
        {"plain_012AZ", "plain_012AZ"},
        {"R", "R"},
        {"Rprefix_09", "Rprefix_09"},
        {"R\\\nname_9", "Rname_9"},
        {"ab\xC3\xA9"
         "CD_9",
         "ab\xC3\xA9"
         "CD_9"},
        {"\xCE\xB1"
         "ab_09",
         "\xCE\xB1"
         "ab_09"},
        {"\xF0\x9F\x98\x80"
         "ab_2",
         "\xF0\x9F\x98\x80"
         "ab_2"},
        {"ab\\\nCD\\\r\nEF", "abCDEF"},
        {"ab\\\n"
         "\xCE\xB1"
         "CD_9",
         "ab\xCE\xB1"
         "CD_9"},
        {"ab\xCE\xB1"
         "\\\r\nCD_9",
         "ab\xCE\xB1"
         "CD_9"},
    };

    // Each input is one complete identifier. The Unicode and splice cases must
    // resume consuming ASCII suffixes rather than returning an early token.
    for (const auto& testCase : cases)
    {
        const auto physical = UnownedStringSlice(testCase.physicalSpelling);
        const auto logical = UnownedStringSlice(testCase.logicalSpelling);
        LexemeTestInput input(physical);
        const auto identifier = input.lexer.readLexeme();
        SLANG_CHECK(identifier.is<RawIdentifier>());
        SLANG_CHECK(identifier.getPhysicalSpelling() == physical);
        SLANG_CHECK(identifier.getLogicalSpelling() == logical);
        SLANG_CHECK(identifier.getName() == input.namePool.getName(logical));
        SLANG_CHECK(identifier.getLoc() == input.beginLoc);
        SLANG_CHECK(
            identifier.getFlags() == (TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace));
        const auto eof = input.lexer.readLexeme();
        SLANG_CHECK(eof.is<RawEndOfFile>());
        SLANG_CHECK(eof.getLoc() == input.beginLoc + physical.getLength());
        SLANG_CHECK(input.sink.getErrorCount() == 0);
    }
}

/// Checks that a splice before punctuation or EOF remains its own atomic lexeme.
SLANG_UNIT_TEST(lexerIdentifierLeavesBoundarySpliceSeparate)
{
    /// An identifier followed by a separate splice and another atomic lexeme.
    struct IdentifierBoundaryCase
    {
        const char* text;
        const char* identifierSpelling;
        const char* spliceSpelling;
        LexemeTypeTag followingKind;
    };
    const IdentifierBoundaryCase cases[] = {
        {"alpha\\\n+", "alpha", "\\\n", LexemeTypeTag::OpAdd},
        {"alpha\\\r\n+", "alpha", "\\\r\n", LexemeTypeTag::OpAdd},
        {"alpha\\\n", "alpha", "\\\n", LexemeTypeTag::EndOfFile},
        {"ab\xC3\xA9"
         "\\\n+",
         "ab\xC3\xA9",
         "\\\n",
         LexemeTypeTag::OpAdd},
    };

    // Peeking through a splice may find punctuation rather than another identifier
    // continuation. Neither the splice nor that punctuation belongs to the identifier.
    for (const auto& testCase : cases)
    {
        LexemeTestInput input(UnownedStringSlice(testCase.text));
        const auto identifierSpelling = UnownedStringSlice(testCase.identifierSpelling);
        const auto spliceSpelling = UnownedStringSlice(testCase.spliceSpelling);
        const auto identifier = input.lexer.readLexeme();
        SLANG_CHECK(identifier.is<RawIdentifier>());
        SLANG_CHECK(identifier.getPhysicalSpelling() == identifierSpelling);
        SLANG_CHECK(identifier.getLogicalSpelling() == identifierSpelling);
        const auto splice = input.lexer.readLexeme();
        SLANG_CHECK(splice.is<EscapedNewLineTrivia>());
        SLANG_CHECK(splice.getPhysicalSpelling() == spliceSpelling);
        SLANG_CHECK(splice.getLogicalSpelling().getLength() == 0);
        SLANG_CHECK(splice.getLoc() == input.beginLoc + identifierSpelling.getLength());
        const auto following = input.lexer.readLexeme();
        SLANG_CHECK(following.getTypeTag() == testCase.followingKind);
        SLANG_CHECK(following.getFlags() == 0);
        SLANG_CHECK(
            following.getLoc() ==
            input.beginLoc + identifierSpelling.getLength() + spliceSpelling.getLength());
        SLANG_CHECK(input.sink.getErrorCount() == 0);
    }
}

/// Checks that identifiers exclude an adjacent identifier byte outside the supplied view.
SLANG_UNIT_TEST(lexerIdentifierRespectsBoundedEnd)
{
    // The final `z` is outside each view. Consuming it would extend the identifier
    // beyond that range, including after the Unicode-to-ASCII transition.
    const char ascii[] = {'a', 'b', '_', '9', 'z'};
    const char unicode[] = {'a', char(0xC3), char(0xA9), 'b', 'z'};
    const UnownedStringSlice inputs[] = {
        UnownedStringSlice(ascii, size_t(4)),
        UnownedStringSlice(unicode, size_t(4)),
    };
    for (auto text : inputs)
    {
        LexemeTestInput input(text);
        const auto identifier = input.lexer.readLexeme();
        SLANG_CHECK(identifier.is<RawIdentifier>());
        SLANG_CHECK(identifier.getPhysicalSpelling() == text);
        SLANG_CHECK(identifier.getLogicalSpelling() == text);
        const auto eof = input.lexer.readLexeme();
        SLANG_CHECK(eof.is<RawEndOfFile>());
        SLANG_CHECK(eof.getLoc() == input.beginLoc + 4);
        SLANG_CHECK(input.sink.getErrorCount() == 0);
    }
}
