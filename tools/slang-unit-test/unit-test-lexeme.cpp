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
        lexer.initialize(view, &sink, &namePool);
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

SLANG_UNIT_TEST(lexemePoolSeparatesSpellingFromOccurrences)
{
    // The same spelling can occur at different locations and with different preprocessing
    // context. Only the returned values differ; both refer to one immutable object.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    const auto spelling = UnownedStringSlice("alpha\\\nbeta");
    const auto content = UnownedStringSlice("alphabeta");
    auto first =
        pool.createLexeme(LexemeKind::Identifier, spelling, content, SourceLoc::fromRaw(100));
    auto second = pool.createLexeme(
        LexemeKind::Identifier,
        spelling,
        content,
        SourceLoc::fromRaw(200),
        TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace);
    SLANG_CHECK(first.getObject() == second.getObject());
    SLANG_CHECK(first.getLoc() == SourceLoc::fromRaw(100));
    SLANG_CHECK(second.getLoc() == SourceLoc::fromRaw(200));
    SLANG_CHECK(first.getFlags() == 0);
    SLANG_CHECK(second.getFlags() == (TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace));

    // Scrubbing does not erase the distinction between physical spellings. Keeping that
    // distinction lets a token stream reproduce the escaped newline exactly.
    auto unspliced = pool.createLexeme(LexemeKind::Identifier, content, content);
    SLANG_CHECK(first.getObject() != unspliced.getObject());
    SLANG_CHECK(first.getContent() == unspliced.getContent());
    SLANG_CHECK(first.getSpelling() == spelling);
    SLANG_CHECK(unspliced.getSpelling() == content);
}

SLANG_UNIT_TEST(lexemePoolOwnsInputText)
{
    // Pool creation must copy caller-owned text. Changing that text afterward must not
    // alter an existing object or make lookup miss the original spelling.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    char spelling[] = "item";
    auto lexeme = pool.createLexeme(
        LexemeKind::Identifier,
        UnownedStringSlice(spelling),
        UnownedStringSlice(spelling));
    spelling[0] = 'X';
    SLANG_CHECK(lexeme.getSpelling() == UnownedStringSlice("item"));
    SLANG_CHECK(lexeme.getContent() == UnownedStringSlice("item"));

    auto repeated = pool.createLexeme(
        LexemeKind::Identifier,
        UnownedStringSlice("item"),
        UnownedStringSlice("item"));
    SLANG_CHECK(lexeme.getObject() == repeated.getObject());
}

SLANG_UNIT_TEST(tokenPoolInternsOrderedTrivia)
{
    // For the physical cluster " /*c*/x\n", the raw token starts six bytes after
    // its leading trivia. A second occurrence shares objects but retains its own context.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    auto space =
        pool.createLexeme(LexemeKind::WhiteSpace, UnownedStringSlice(" "), UnownedStringSlice(" "));
    auto comment = pool.createLexeme(
        LexemeKind::BlockComment,
        UnownedStringSlice("/*c*/"),
        UnownedStringSlice("/*c*/"));
    auto newline =
        pool.createLexeme(LexemeKind::NewLine, UnownedStringSlice("\n"), UnownedStringSlice("\n"));
    const LexemeObj* leading[] = {space.getObject(), comment.getObject()};
    const LexemeObj* trailing[] = {newline.getObject()};
    auto raw = pool.createLexeme(
        LexemeKind::Identifier,
        UnownedStringSlice("x"),
        UnownedStringSlice("x"),
        SourceLoc::fromRaw(100));
    auto token = pool.createToken(
        raw.as<RawToken>(),
        makeConstArrayView(leading),
        makeConstArrayView(trailing));
    auto otherRaw = pool.createLexeme(
        LexemeKind::Identifier,
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
    SLANG_CHECK(number.is<LiteralToken>());
    SLANG_CHECK(number.is<Token>());
    SLANG_CHECK(number.getRawToken().is<RawNumericLiteral>());
    SLANG_CHECK(number.getRawToken().is<Lexeme>());
    SLANG_CHECK(!number.is<IdentifierToken>());
    SLANG_CHECK(number.as<IntegerLiteralToken>().getObject() == number.getObject());
}

SLANG_UNIT_TEST(tokenReplacementPreservesSharedObject)
{
    // A replacement is a fresh factory request. Other occurrences of the original
    // token must continue to refer to the original immutable object.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    auto original = pool.createToken(TokenType::Identifier, UnownedStringSlice("before"));
    auto sharedOriginal = original;
    auto replaced = pool.createToken(TokenType::Identifier, UnownedStringSlice("after"));
    SLANG_CHECK(original.getContent() == UnownedStringSlice("before"));
    SLANG_CHECK(sharedOriginal.getObject() == original.getObject());
    SLANG_CHECK(sharedOriginal.getContent() == UnownedStringSlice("before"));
    SLANG_CHECK(replaced.getContent() == UnownedStringSlice("after"));
    SLANG_CHECK(original.getObject() != replaced.getObject());

    auto repeated = pool.createToken(TokenType::Identifier, UnownedStringSlice("after"));
    SLANG_CHECK(replaced.getObject() == repeated.getObject());
    auto flagged = replaced.withFlags(TokenFlag::AfterWhitespace);
    SLANG_CHECK(flagged.getObject() == repeated.getObject());
    SLANG_CHECK(flagged.getFlags() == TokenFlag::AfterWhitespace);
    SLANG_CHECK(replaced.getFlags() == 0);
    SLANG_CHECK(repeated.getFlags() == 0);
}

SLANG_UNIT_TEST(tokenReaderTypedConditionalRead)
{
    // A mismatched conditional read leaves the cursor untouched. A matching read
    // returns the requested type and advances to the following token.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    TokenList tokens;
    tokens.add(pool.createToken(TokenType::Identifier, UnownedStringSlice("value")));
    tokens.add(pool.createToken(TokenType::IntegerLiteral, UnownedStringSlice("12")));
    tokens.add(pool.createEndOfFile());
    TokenReader reader(tokens);
    auto start = reader.getCursor();
    SLANG_CHECK(!reader.advanceIf<IntegerLiteralToken>());
    SLANG_CHECK(reader.isAtCursor(start));
    auto identifier = reader.advanceIf<IdentifierToken>();
    SLANG_CHECK(bool(identifier));
    SLANG_CHECK(identifier.getContent() == UnownedStringSlice("value"));
    SLANG_CHECK(reader.peekTokenType() == TokenType::IntegerLiteral);

    // A conditional read can use an abstract token group, and a successful EOF read
    // remains distinguishable from the empty result of a mismatched read.
    auto literal = reader.advanceIf<LiteralToken>();
    SLANG_CHECK(literal.getContent() == UnownedStringSlice("12"));
    SLANG_CHECK(bool(reader.advanceIf<EndOfFileToken>()));
    SLANG_CHECK(reader.isAtEnd());
    SLANG_CHECK(!reader.advanceIf<IdentifierToken>());
}

SLANG_UNIT_TEST(tokenSplitPreservesPhysicalOffsetsAndTrailingTrivia)
{
    // The first `>` is followed by a backslash, a newline, and the second `>`.
    // Splitting uses the physical offset of that second character, three bytes after
    // the first one, rather than the offset within the logical spelling `>>`.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    auto rightShift = pool.createLexeme(
        LexemeKind::OpRsh,
        UnownedStringSlice(">\\\n>"),
        UnownedStringSlice(">>"),
        SourceLoc::fromRaw(100),
        TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace);
    auto comment = pool.createLexeme(
        LexemeKind::LineComment,
        UnownedStringSlice("//tail"),
        UnownedStringSlice("//tail"));
    const LexemeObj* trailing[] = {comment.getObject()};
    auto original = pool.createToken(rightShift.as<RawToken>(), {}, makeConstArrayView(trailing));
    auto second = pool.createSecondGreaterToken(rightShift.as<RawOpRsh>());
    SLANG_CHECK(second.getType() == TokenType::OpGreater);
    SLANG_CHECK(second.getSpelling() == UnownedStringSlice(">"));
    SLANG_CHECK(second.getContent() == UnownedStringSlice(">"));
    SLANG_CHECK(second.getLoc() == SourceLoc::fromRaw(103));
    SLANG_CHECK(second.getFlags() == 0);
    SLANG_CHECK(rightShift.getContent() == UnownedStringSlice(">>"));

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

SLANG_UNIT_TEST(tokenRefRetainsPoolAndName)
{
    // Clients without an AST can retain a typed token after both the name pool and
    // the original pool reference go away. The token pool must retain its `Name` objects.
    bool wasPoolDestroyed = false;
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
    SLANG_CHECK(retained->getContent() == UnownedStringSlice("retained"));
    SLANG_CHECK_ABORT(retained->getName() != nullptr);
    SLANG_CHECK(retained->getName()->text == "retained");
    retained = TokenRef<IdentifierToken>();
    SLANG_CHECK(wasPoolDestroyed);
}

SLANG_UNIT_TEST(tokenPoolImportOwnsForeignTrivia)
{
    // Imported clusters must depend only on the destination pool. We release every
    // owner of the original cluster before reading the imported spelling and trivia.
    RefPtr<TokenPool> destinationOwner = new TokenPool();
    auto& destination = *destinationOwner;
    Token imported;
    {
        RefPtr<TokenPool> originalOwner = new TokenPool();
        auto& original = *originalOwner;
        auto raw = original.createLexeme(
            LexemeKind::IntegerLiteral,
            UnownedStringSlice("42"),
            UnownedStringSlice("42"),
            SourceLoc::fromRaw(20),
            TokenFlag::AfterWhitespace);
        auto comment = original.createLexeme(
            LexemeKind::BlockComment,
            UnownedStringSlice("/*c*/"),
            UnownedStringSlice("/*c*/"));
        const LexemeObj* trivia[] = {comment.getObject()};
        auto token = original.createToken(raw.as<RawToken>(), makeConstArrayView(trivia));
        imported = destination.importToken(token);
        SLANG_CHECK(imported.getObject() != token.getObject());
        SLANG_CHECK(imported.getRawToken().getPool() == &destination);
        SLANG_CHECK_ABORT(imported.getLeadingTrivia().getCount() == 1);
        SLANG_CHECK(imported.getLeadingTrivia()[0].getPool() == &destination);
    }
    SLANG_CHECK(imported.getContent() == UnownedStringSlice("42"));
    SLANG_CHECK(imported.getLeadingTrivia()[0].getSpelling() == UnownedStringSlice("/*c*/"));
    SLANG_CHECK(imported.getLeadingTrivia()[0].getLoc() == SourceLoc::fromRaw(15));
    SLANG_CHECK(imported.getFlags() == TokenFlag::AfterWhitespace);
}

SLANG_UNIT_TEST(tokenPoolEmptyImportPreservesOccurrenceState)
{
    // A null object makes the token empty, independently of location and context.
    // Importing that value must preserve the independent occurrence fields as well.
    RefPtr<TokenPool> pool = new TokenPool();
    const auto loc = SourceLoc::fromRaw(150);
    const TokenFlags flags = TokenFlag::AtStartOfLine | TokenFlag::AfterWhitespace;
    Token empty(nullptr, loc, flags);
    auto imported = pool->importToken(empty);
    SLANG_CHECK(!imported);
    SLANG_CHECK(imported.getObject() == nullptr);
    SLANG_CHECK(imported.getLoc() == loc);
    SLANG_CHECK(imported.getFlags() == flags);
}

SLANG_UNIT_TEST(tokenListReplacementRetainsEveryPool)
{
    // A list can alternate between pools, and the original owners can disappear
    // before the list replaces itself. Destruction flags make lifetime failures
    // observable before any test attempts to read the borrowed spelling.
    bool wasFirstPoolDestroyed = false;
    bool wasSecondPoolDestroyed = false;
    {
        TokenList tokens;
        {
            RefPtr<TokenPool> first = new TokenPoolWithDestructionFlag(wasFirstPoolDestroyed);
            RefPtr<TokenPool> second = new TokenPoolWithDestructionFlag(wasSecondPoolDestroyed);
            List<Token> replacement;
            replacement.add(first->createToken(TokenType::Identifier, UnownedStringSlice("one")));
            replacement.add(
                second->createToken(TokenType::IntegerLiteral, UnownedStringSlice("2")));
            replacement.add(first->createToken(TokenType::Identifier, UnownedStringSlice("three")));
            replacement.add(second->createEndOfFile());
            tokens.setTokens(replacement);
        }
        SLANG_CHECK_ABORT(!wasFirstPoolDestroyed);
        SLANG_CHECK_ABORT(!wasSecondPoolDestroyed);

        // Self replacement aliases the list's own storage. Retaining replacement pools
        // must precede releasing old values, even though no external owner remains.
        tokens.setTokens(tokens.getTokens());
        SLANG_CHECK_ABORT(!wasFirstPoolDestroyed);
        SLANG_CHECK_ABORT(!wasSecondPoolDestroyed);
        TokenReader reader(tokens);
        SLANG_CHECK(reader.advanceIf<IdentifierToken>().getContent() == UnownedStringSlice("one"));
        SLANG_CHECK(
            reader.advanceIf<IntegerLiteralToken>().getContent() == UnownedStringSlice("2"));
        SLANG_CHECK(
            reader.advanceIf<IdentifierToken>().getContent() == UnownedStringSlice("three"));
        SLANG_CHECK(reader.peekTokenType() == TokenType::EndOfFile);
    }
    SLANG_CHECK(wasFirstPoolDestroyed);
    SLANG_CHECK(wasSecondPoolDestroyed);
}

SLANG_UNIT_TEST(tokenListRetainsLexicalStorage)
{
    // A saved token list can outlive the lexer, the name pool, and the `SourceManager`.
    // Its pool references must preserve spelling and names without borrowing file bytes.
    TokenList tokens;
    {
        LexemeTestInput input("one //tail\n");
        tokens = input.lexer.readAllTokens();
        SLANG_CHECK(input.sink.getErrorCount() == 0);
    }

    TokenReader reader(tokens);
    auto first = reader.advanceIf<IdentifierToken>();
    SLANG_CHECK(first.getContent() == UnownedStringSlice("one"));
    SLANG_CHECK_ABORT(first.getName() != nullptr);
    SLANG_CHECK(first.getName()->text == "one");
    SLANG_CHECK_ABORT(first.getTrailingTrivia().getCount() == 3);
    SLANG_CHECK(first.getTrailingTrivia()[1].getSpelling() == UnownedStringSlice("//tail"));
    SLANG_CHECK(reader.peekTokenType() == TokenType::EndOfFile);
}

SLANG_UNIT_TEST(tokenTriviaPreservesUnknownSourceLocations)
{
    // A token reconstructed without debug locations can still have spelling and
    // trivia. Neither indexing nor iteration may invent locations from text lengths.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    auto raw = pool.createLexeme(
        LexemeKind::IntegerLiteral,
        UnownedStringSlice("42"),
        UnownedStringSlice("42"));
    auto space =
        pool.createLexeme(LexemeKind::WhiteSpace, UnownedStringSlice(" "), UnownedStringSlice(" "));
    auto comment = pool.createLexeme(
        LexemeKind::BlockComment,
        UnownedStringSlice("/*c*/"),
        UnownedStringSlice("/*c*/"));
    auto newline =
        pool.createLexeme(LexemeKind::NewLine, UnownedStringSlice("\n"), UnownedStringSlice("\n"));
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
        reconstructed << trivia.getSpelling();
    }
    reconstructed << token.getSpelling();
    for (auto trivia : token.getTrailingTrivia())
    {
        SLANG_CHECK(!trivia.getLoc().isValid());
        reconstructed << trivia.getSpelling();
    }
    SLANG_CHECK(reconstructed.getUnownedSlice() == UnownedStringSlice(" /*c*/42\n "));
}

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
            reconstructed << lexeme.getSpelling();
            offset += lexeme.getSpellingLength();
        }
        SLANG_CHECK(lexemes.end()->is<RawEndOfFile>());
        SLANG_CHECK(lexemes.end()->getLoc() == input.beginLoc + offset);
        SLANG_CHECK(reconstructed.getUnownedSlice() == UnownedStringSlice(text));
        SLANG_CHECK(input.sink.getErrorCount() == 0);
    }
}

SLANG_UNIT_TEST(lexerFinalBackslashUsesBufferEnd)
{
    // The byte after the view is deliberately nonzero. The final backslash is an
    // invalid character, and the lexer must not treat the excluded byte as input.
    const char bytes[] = {'\\', 'x'};
    LexemeTestInput input(UnownedStringSlice(bytes, size_t(1)));
    SLANG_CHECK(input.lexer.m_end == bytes + 1);
    auto backslash = input.lexer.readLexeme();
    SLANG_CHECK(backslash.is<RawInvalid>());
    SLANG_CHECK(backslash.getSpelling() == UnownedStringSlice("\\"));
    auto eof = input.lexer.readLexeme();
    SLANG_CHECK(eof.is<RawEndOfFile>());
    SLANG_CHECK(eof.getLoc() == input.beginLoc + 1);
    SLANG_CHECK(input.sink.getErrorCount() == 1);
}

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
        SLANG_CHECK(identifier.getSpelling() == UnownedStringSlice("a"));
        auto splice = input.lexer.readLexeme();
        SLANG_CHECK(splice.is<EscapedNewLineTrivia>());
        SLANG_CHECK(splice.getSpelling() == UnownedStringSlice(bytes + 1, bytes + 4));
        SLANG_CHECK(splice.getContent().getLength() == 0);
        auto eof = input.lexer.readLexeme();
        SLANG_CHECK(eof.is<RawEndOfFile>());
        SLANG_CHECK(eof.getLoc() == input.beginLoc + 4);
        SLANG_CHECK(input.sink.getErrorCount() == 0);
    }
}

SLANG_UNIT_TEST(lexerTruncatedUtf8UsesBufferEnd)
{
    // E2 82 begins a three-byte UTF-8 sequence. The third byte lies outside the view,
    // so recognition must diagnose the incomplete sequence and stop at the view's end.
    const char bytes[] = {char(0xE2), char(0x82), char(0xAC)};
    LexemeTestInput input(UnownedStringSlice(bytes, size_t(2)));
    auto invalidSequence = input.lexer.readLexeme();
    SLANG_CHECK(invalidSequence.getSpelling() == UnownedStringSlice(bytes, size_t(2)));
    SLANG_CHECK(input.sink.getErrorCount() == 1);
    auto eof = input.lexer.readLexeme();
    SLANG_CHECK(eof.is<RawEndOfFile>());
    SLANG_CHECK(eof.getLoc() == input.beginLoc + 2);
}

SLANG_UNIT_TEST(lexerInvalidUtf8PreservesFollowingAscii)
{
    // The ASCII `b` cannot continue E2 82. After diagnosing those two bytes, the
    // lexer must leave `b` for the next identifier rather than consume it as UTF-8.
    const char bytes[] = {char(0xE2), char(0x82), 'b'};
    LexemeTestInput input(UnownedStringSlice(bytes, sizeof(bytes)));
    auto invalidSequence = input.lexer.readLexeme();
    SLANG_CHECK(invalidSequence.getSpelling() == UnownedStringSlice(bytes, size_t(2)));
    SLANG_CHECK(input.sink.getErrorCount() == 1);
    auto identifier = input.lexer.readLexeme();
    SLANG_CHECK(identifier.is<RawIdentifier>());
    SLANG_CHECK(identifier.getContent() == UnownedStringSlice("b"));
    SLANG_CHECK(identifier.getLoc() == input.beginLoc + 2);
    SLANG_CHECK(input.lexer.readLexeme().is<RawEndOfFile>());
    SLANG_CHECK(input.sink.getErrorCount() == 1);
}

SLANG_UNIT_TEST(lexerInvalidUnicodeCannotBecomeEndOfFile)
{
    // The extended seven-byte encoding denotes FFFFFFFF, which becomes -1 if cast
    // to a signed 32-bit integer. The lexer must reject that code point before it
    // can be confused with EOF, preserving both the invalid bytes and the next token.
    const char bytes[] =
        {char(0xFE), char(0x83), char(0xBF), char(0xBF), char(0xBF), char(0xBF), char(0xBF), 'x'};
    LexemeTestInput input(UnownedStringSlice(bytes, sizeof(bytes)));
    auto invalidSequence = input.lexer.readLexeme();
    SLANG_CHECK(invalidSequence.is<WhiteSpaceTrivia>());
    SLANG_CHECK(invalidSequence.getSpelling() == UnownedStringSlice(bytes, size_t(7)));
    SLANG_CHECK(input.sink.getErrorCount() == 1);
    auto identifier = input.lexer.readLexeme();
    SLANG_CHECK(identifier.is<RawIdentifier>());
    SLANG_CHECK(identifier.getContent() == UnownedStringSlice("x"));
    auto eof = input.lexer.readLexeme();
    SLANG_CHECK(eof.is<RawEndOfFile>());
    SLANG_CHECK(eof.getLoc() == input.beginLoc + sizeof(bytes));
    SLANG_CHECK(input.sink.getErrorCount() == 1);

    StringBuilder reconstructed;
    reconstructed << invalidSequence.getSpelling() << identifier.getSpelling() << eof.getSpelling();
    SLANG_CHECK(reconstructed.getUnownedSlice() == UnownedStringSlice(bytes, sizeof(bytes)));
}

SLANG_UNIT_TEST(lexerAcceptsMaximumUnicodeScalar)
{
    // U+10FFFF is the largest Unicode scalar value. The lexer accepts non-ASCII
    // scalar values in identifiers, including this last value in the valid range.
    const char bytes[] = {char(0xF4), char(0x8F), char(0xBF), char(0xBF)};
    LexemeTestInput input(UnownedStringSlice(bytes, sizeof(bytes)));
    auto identifier = input.lexer.readLexeme();
    SLANG_CHECK(identifier.is<RawIdentifier>());
    SLANG_CHECK(identifier.getSpelling() == UnownedStringSlice(bytes, sizeof(bytes)));
    SLANG_CHECK(input.lexer.readLexeme().is<RawEndOfFile>());
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

SLANG_UNIT_TEST(lexerRejectsUnicodeSurrogate)
{
    // ED A0 80 encodes the surrogate U+D800, which is not a Unicode scalar value.
    // Recovery must consume those three bytes and leave the ASCII identifier intact.
    const char bytes[] = {char(0xED), char(0xA0), char(0x80), 'x'};
    LexemeTestInput input(UnownedStringSlice(bytes, sizeof(bytes)));
    auto invalidSequence = input.lexer.readLexeme();
    SLANG_CHECK(invalidSequence.is<WhiteSpaceTrivia>());
    SLANG_CHECK(invalidSequence.getSpelling() == UnownedStringSlice(bytes, size_t(3)));
    SLANG_CHECK(input.sink.getErrorCount() == 1);
    auto identifier = input.lexer.readLexeme();
    SLANG_CHECK(identifier.is<RawIdentifier>());
    SLANG_CHECK(identifier.getContent() == UnownedStringSlice("x"));
    SLANG_CHECK(input.lexer.readLexeme().is<RawEndOfFile>());
    SLANG_CHECK(input.sink.getErrorCount() == 1);
}

SLANG_UNIT_TEST(lexerRetainsOriginalAndLogicalSpellings)
{
    // Identifier lookup uses the spelling after line splicing, while reconstruction
    // uses the original CRLF continuation. Neither operation may overwrite the other.
    LexemeTestInput input("alpha\\\r\nbeta");
    auto identifier = input.lexer.readLexeme();
    SLANG_CHECK(identifier.is<RawIdentifier>());
    SLANG_CHECK(identifier.getSpelling() == UnownedStringSlice("alpha\\\r\nbeta"));
    SLANG_CHECK(identifier.getContent() == UnownedStringSlice("alphabeta"));
    SLANG_CHECK(identifier.getName() == input.namePool.getName(String("alphabeta")));
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

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
    SLANG_CHECK(first.getContent() == UnownedStringSlice("first"));
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
            reconstructed << trivia.getSpelling();
        reconstructed << token.getSpelling();
        for (auto trivia : token.getTrailingTrivia())
            reconstructed << trivia.getSpelling();
    }
    SLANG_CHECK(reconstructed.getUnownedSlice() == UnownedStringSlice(text));
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

SLANG_UNIT_TEST(lexerTriviaOnlyFileUsesEndOfFileToken)
{
    // With no preceding token, every comment and newline belongs to EOF's leading
    // trivia. EOF remains a token even when all bytes in the file are trivia.
    const char* text = " //only\n\t/*last*/";
    LexemeTestInput input(text);
    auto eof = input.lexer.readToken();
    SLANG_CHECK(eof.is<EndOfFileToken>());
    SLANG_CHECK(eof.getLeadingTrivia().getCount() == 5);
    SLANG_CHECK(eof.getTrailingTrivia().getCount() == 0);
    StringBuilder reconstructed;
    for (auto trivia : eof.getLeadingTrivia())
        reconstructed << trivia.getSpelling();
    SLANG_CHECK(reconstructed.getUnownedSlice() == UnownedStringSlice(text));
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

SLANG_UNIT_TEST(tokenReaderPreservesStoredEndOfFileTrivia)
{
    // The first newline trails `a`; the remaining newline and comment lead EOF.
    // Reading the list must return that exact EOF occurrence, including its trivia.
    LexemeTestInput input("a\n\n//last");
    auto tokens = input.lexer.readAllTokens();
    const auto storedEOF = *tokens.end();
    SLANG_CHECK_ABORT(storedEOF.is<EndOfFileToken>());
    SLANG_CHECK_ABORT(storedEOF.getLeadingTrivia().getCount() == 2);
    SLANG_CHECK(storedEOF.getLeadingTrivia()[0].getSpelling() == UnownedStringSlice("\n"));
    SLANG_CHECK(storedEOF.getLeadingTrivia()[1].getSpelling() == UnownedStringSlice("//last"));
    TokenReader reader(tokens);
    SLANG_CHECK(reader.advanceIf<IdentifierToken>().getContent() == UnownedStringSlice("a"));

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

SLANG_UNIT_TEST(tokenReaderBoundedRangeUsesExplicitEndOfFile)
{
    // The range excludes `two`. Its supplied EOF has a different location and flags
    // from the file's EOF, so the reader cannot derive its endpoint from either token.
    LexemeTestInput input("one two");
    auto tokens = input.lexer.readAllTokens();
    RefPtr<TokenPool> pool = new TokenPool();
    auto endpoint = pool->createEndOfFile(SourceLoc::fromRaw(150), TokenFlag::AfterWhitespace);
    TokenReader reader(tokens.begin(), tokens.begin() + 1, endpoint);
    SLANG_CHECK(reader.advanceIf<IdentifierToken>().getContent() == UnownedStringSlice("one"));
    SLANG_CHECK(reader.isAtEnd());
    for (int read = 0; read < 3; ++read)
    {
        auto eof = reader.readToken();
        SLANG_CHECK(eof.getObject() == endpoint.getObject());
        SLANG_CHECK(eof.getLoc() == endpoint.getLoc());
        SLANG_CHECK(eof.getFlags() == endpoint.getFlags());
    }
}

SLANG_UNIT_TEST(tokenReaderEmptyRangeUsesExplicitEndOfFile)
{
    // An empty range needs no backing array. Its explicit EOF remains readable and
    // stable because the reader does not need to dereference either range pointer.
    RefPtr<TokenPool> pool = new TokenPool();
    auto endpoint = pool->createEndOfFile(SourceLoc::fromRaw(150));
    TokenReader reader(nullptr, nullptr, endpoint);
    SLANG_CHECK(reader.isAtEnd());
    SLANG_CHECK(reader.getCount() == 0);
    SLANG_CHECK(reader.peekToken().getObject() == endpoint.getObject());
    SLANG_CHECK(reader.readToken().getLoc() == endpoint.getLoc());
    SLANG_CHECK(reader.readToken().getObject() == endpoint.getObject());
}

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
    SLANG_CHECK(restored.getTrailingTrivia()[1].getSpelling() == UnownedStringSlice("//tail"));
    SLANG_CHECK(reader.advanceIf<IdentifierToken>().getContent() == UnownedStringSlice("next"));

    reader.setCursor(beforeSplit);
    SLANG_CHECK(reader.peekToken().getObject() == original.getObject());
    SLANG_CHECK(reader.readToken().is<OpRshToken>());
    SLANG_CHECK(reader.is<IdentifierToken>());
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

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

SLANG_UNIT_TEST(lexerAngleBracketHeaderNameModePrecedesAdjacentOperand)
{
    // There is no space between "include" and its operand. Switching mode after
    // receiving the identifier must still recognize the complete angle-bracket name,
    // including characters that would otherwise start a C-style comment.
    LexemeTestInput input("#include<one/*name*/two.h>\nnext");
    SLANG_CHECK(input.lexer.readToken().is<PoundToken>());
    SLANG_CHECK(input.lexer.readToken().getContent() == UnownedStringSlice("include"));
    input.lexer.setMode(LexerMode::ExpectingAngleBracketHeaderName);
    auto header = input.lexer.readToken();
    SLANG_CHECK(header.is<AngleBracketHeaderNameToken>());
    SLANG_CHECK(header.getSpelling() == UnownedStringSlice("<one/*name*/two.h>"));
    input.lexer.setMode(LexerMode::Normal);
    SLANG_CHECK(input.lexer.readToken().getContent() == UnownedStringSlice("next"));
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

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
