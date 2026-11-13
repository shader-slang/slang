// These tests exercise the contracts between atomic lexing, trivia attachment, and borrowed
// token values. We first check interning and ownership without a lexer, then reconstruct
// physical input and exercise the boundaries where preprocessing changes lexer state.
// We allocate pools under `RefPtr` because `TokenList` and `TokenRef` retain pool references.

#include "compiler-core/slang-lexer.h"
#include "compiler-core/slang-token.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

/// A lexer with storage for one input file and its diagnostic locations.
struct LexemeTestInput
{
    SourceManager sourceManager;
    NamePool namePool;
    DiagnosticSink sink;
    Lexer lexer;
    SourceLoc beginLoc;

    explicit LexemeTestInput(const char* text)
        : sink(&sourceManager, nullptr)
    {
        // We keep the file and its view together so byte offsets in the assertions refer
        // to the same location range that the lexer uses.
        sourceManager.initialize(nullptr, nullptr);
        auto file = sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), String(text));
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
    // Replacing one occurrence must not mutate a shared object. Repeating the same
    // replacement through the pool must then find the same immutable replacement object.
    RefPtr<TokenPool> poolOwner = new TokenPool();
    auto& pool = *poolOwner;
    auto original = pool.createToken(TokenType::Identifier, UnownedStringSlice("before"));
    auto replaced = original;
    replaced.setContent(UnownedStringSlice("after"));
    SLANG_CHECK(original.getContent() == UnownedStringSlice("before"));
    SLANG_CHECK(replaced.getContent() == UnownedStringSlice("after"));
    SLANG_CHECK(original.getObject() != replaced.getObject());

    auto repeated = pool.createToken(TokenType::Identifier, UnownedStringSlice("after"));
    SLANG_CHECK(replaced.getObject() == repeated.getObject());
    replaced.setFlags(TokenFlag::AfterWhitespace);
    SLANG_CHECK(replaced.getObject() == repeated.getObject());
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
    auto second = pool.createSecondGreaterToken(rightShift.as<RawToken>());
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
    SLANG_CHECK(!pool.createSecondGreaterToken(unlocated.getRawToken()).getLoc().isValid());
}

SLANG_UNIT_TEST(tokenRefRetainsPoolAndName)
{
    // Clients without an AST can retain a typed token after both the name pool and
    // the original pool reference go away. The token pool must retain its `Name` objects.
    bool poolDestroyed = false;
    TokenRef<IdentifierToken> retained;
    {
        NamePool names;
        RefPtr<TokenPool> pool = new TokenPoolWithDestructionFlag(poolDestroyed);
        auto name = names.getName(String("retained"));
        auto token = pool->createToken(TokenType::Identifier, name).as<IdentifierToken>();
        retained = TokenRef<IdentifierToken>(token);
    }
    SLANG_CHECK_ABORT(!poolDestroyed);
    SLANG_CHECK(bool(retained));
    SLANG_CHECK(retained->getContent() == UnownedStringSlice("retained"));
    SLANG_CHECK_ABORT(retained->getName() != nullptr);
    SLANG_CHECK(retained->getName()->text == "retained");
    retained = TokenRef<IdentifierToken>();
    SLANG_CHECK(poolDestroyed);
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
    bool firstPoolDestroyed = false;
    bool secondPoolDestroyed = false;
    {
        TokenList tokens;
        {
            RefPtr<TokenPool> first = new TokenPoolWithDestructionFlag(firstPoolDestroyed);
            RefPtr<TokenPool> second = new TokenPoolWithDestructionFlag(secondPoolDestroyed);
            List<Token> replacement;
            replacement.add(first->createToken(TokenType::Identifier, UnownedStringSlice("one")));
            replacement.add(
                second->createToken(TokenType::IntegerLiteral, UnownedStringSlice("2")));
            replacement.add(first->createToken(TokenType::Identifier, UnownedStringSlice("three")));
            replacement.add(second->createEndOfFile());
            tokens.setTokens(replacement);
        }
        SLANG_CHECK_ABORT(!firstPoolDestroyed);
        SLANG_CHECK_ABORT(!secondPoolDestroyed);

        // Self replacement aliases the list's own storage. Retaining replacement pools
        // must precede releasing old values, even though no external owner remains.
        tokens.setTokens(tokens.getTokens());
        SLANG_CHECK_ABORT(!firstPoolDestroyed);
        SLANG_CHECK_ABORT(!secondPoolDestroyed);
        TokenReader reader(tokens);
        SLANG_CHECK(reader.advanceIf<IdentifierToken>().getContent() == UnownedStringSlice("one"));
        SLANG_CHECK(
            reader.advanceIf<IntegerLiteralToken>().getContent() == UnownedStringSlice("2"));
        SLANG_CHECK(
            reader.advanceIf<IdentifierToken>().getContent() == UnownedStringSlice("three"));
        SLANG_CHECK(reader.peekTokenType() == TokenType::EndOfFile);
    }
    SLANG_CHECK(firstPoolDestroyed);
    SLANG_CHECK(secondPoolDestroyed);
}

SLANG_UNIT_TEST(tokenListRetainsLexicalStorage)
{
    // A saved token list can outlive the lexer, the name pool, and the `SourceManager`.
    // Its pool references must preserve spelling and names without borrowing file bytes.
    TokenList tokens;
    {
        LexemeTestInput input("one //tail\n");
        tokens = input.lexer.lexAllSemanticTokens();
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
        auto lexemes = input.lexer.lexAllLexemes();
        SLANG_CHECK_ABORT(lexemes.begin() != nullptr);
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

SLANG_UNIT_TEST(lexerRetainsOriginalAndLogicalSpellings)
{
    // Identifier lookup uses the spelling after line splicing, while reconstruction
    // uses the original CRLF continuation. Neither operation may overwrite the other.
    LexemeTestInput input("alpha\\\r\nbeta");
    auto identifier = input.lexer.lexLexeme();
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
    auto first = input.lexer.lexToken();
    auto second = input.lexer.lexToken();
    auto eof = input.lexer.lexToken();
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
    auto eof = input.lexer.lexToken();
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
    // A reader at the real list endpoint must preserve that stored token rather
    // than synthesize a trivia-free EOF intended for the end of a subrange.
    LexemeTestInput input("a\n\n//last");
    auto tokens = input.lexer.lexAllSemanticTokens();
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
        auto eof = reader.advanceToken();
        SLANG_CHECK(eof.getObject() == storedEOF.getObject());
        SLANG_CHECK(eof.getLoc() == storedEOF.getLoc());
        SLANG_CHECK(eof.getFlags() == storedEOF.getFlags());
        SLANG_CHECK_ABORT(eof.getLeadingTrivia().getCount() == 2);
        SLANG_CHECK(eof.getLeadingTrivia()[1].getLoc() == storedEOF.getLeadingTrivia()[1].getLoc());
    }
}

SLANG_UNIT_TEST(lexerTrailingTriviaDoesNotReadNextRawToken)
{
    // The next string literal is malformed. Reading the identifier may consume its
    // trailing space, but must leave both recognition and diagnostics for the string
    // until the caller asks for the next token.
    LexemeTestInput input("name \"unterminated\n");
    auto identifier = input.lexer.lexToken();
    SLANG_CHECK(identifier.is<IdentifierToken>());
    SLANG_CHECK(input.sink.getErrorCount() == 0);
    input.lexer.lexToken();
    SLANG_CHECK(input.sink.getErrorCount() > 0);
}

SLANG_UNIT_TEST(lexerTrailingTriviaStopsBeforeFollowingLine)
{
    // A preprocessor can change diagnostic policy after reading the complete directive
    // line. Even indentation and a comment on the next line must remain unlexed until
    // that change; the malformed string must then obey the new suppression policy.
    LexemeTestInput input("directive\n /*next*/ \"unterminated\n");
    auto directive = input.lexer.lexToken();
    SLANG_CHECK(directive.getTrailingTrivia().getCount() == 1);
    SLANG_CHECK(
        UnownedStringSlice(input.lexer.m_begin, input.lexer.m_cursor) ==
        UnownedStringSlice("directive\n"));
    SLANG_CHECK(input.sink.getErrorCount() == 0);
    input.lexer.m_lexerFlags |= kLexerFlag_SuppressDiagnostics;
    SLANG_CHECK(input.lexer.lexToken().is<StringLiteralToken>());
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}

SLANG_UNIT_TEST(lexerHeaderNameModePrecedesAdjacentOperand)
{
    // There is no space between "include" and its operand. Switching mode after
    // receiving the identifier must still recognize the complete angle-bracket name,
    // including characters that would otherwise start a C-style comment.
    LexemeTestInput input("#include<one/*name*/two.h>\nnext");
    SLANG_CHECK(input.lexer.lexToken().is<PoundToken>());
    SLANG_CHECK(input.lexer.lexToken().getContent() == UnownedStringSlice("include"));
    input.lexer.setHeaderNameMode(true);
    auto header = input.lexer.lexToken();
    SLANG_CHECK(header.is<HeaderNameToken>());
    SLANG_CHECK(header.getSpelling() == UnownedStringSlice("<one/*name*/two.h>"));
    input.lexer.setHeaderNameMode(false);
    SLANG_CHECK(input.lexer.lexToken().getContent() == UnownedStringSlice("next"));
    SLANG_CHECK(input.sink.getErrorCount() == 0);
}
