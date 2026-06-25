// unit-test-token-reader.cpp
//
// Contract under test
// -------------------
// `Slang::Misc::TokenReader` is a small tokenizer used by the
// SPIR-V intrinsic snippet parser (`slang-ir-spirv-snippet.cpp`)
// and similar internal text formats. It is *not* the main Slang
// lexer. The contract:
//
//   * Tokenization happens eagerly in the constructor — the input
//     string is split into a list of tokens, each carrying type,
//     content and position.
//   * Identifiers, integer / float / string literals, and the
//     punctuation set listed in `TokenType` each get their own
//     token type. Operators (`+`, `==`, `<<`, etc.) are
//     pre-classified.
//   * `ReadInt` / `ReadUInt` / `ReadDouble` / `ReadFloat` /
//     `ReadWord` / `ReadStringLiteral` / `Read(expectedStr)` /
//     `Read(TokenType)` consume one token and assert its type;
//     mismatches throw `TextFormatException`. Reading past the end
//     also throws.
//   * `LookAhead` / `NextToken` are non-consuming. `AdvanceIf`
//     consumes only on match.
//   * `Back(n)` rewinds N tokens so a caller can re-read.
//   * `Position.Line` is monotonically non-decreasing as tokens
//     are produced (a token after a newline is on a later line).

#include "../../source/core/slang-token-reader.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;
using namespace Slang::Misc;

SLANG_UNIT_TEST(tokenReaderEmptyInput)
{
    TokenReader r("");
    SLANG_CHECK(r.IsEnd());
    // NextToken() at end returns a default-constructed Token with
    // Type == Unknown.
    SLANG_CHECK(r.NextToken().Type == TokenType::Unknown);
}

SLANG_UNIT_TEST(tokenReaderIdentifiers)
{
    TokenReader r("foo bar_baz Qux2");
    SLANG_CHECK(r.ReadWord() == "foo");
    SLANG_CHECK(r.ReadWord() == "bar_baz");
    SLANG_CHECK(r.ReadWord() == "Qux2");
    SLANG_CHECK(r.IsEnd());
}

SLANG_UNIT_TEST(tokenReaderIntLiterals)
{
    TokenReader r("0 42 -7 +123");
    SLANG_CHECK(r.ReadInt() == 0);
    SLANG_CHECK(r.ReadInt() == 42);
    SLANG_CHECK(r.ReadInt() == -7);
    // Leading + reads as a separate token; ReadInt only handles -.
    // Re-establish a fresh reader for that case.
    TokenReader r2("123");
    SLANG_CHECK(r2.ReadInt() == 123);
}

SLANG_UNIT_TEST(tokenReaderUIntLiterals)
{
    TokenReader r("0 42 4000000000");
    SLANG_CHECK(r.ReadUInt() == 0);
    SLANG_CHECK(r.ReadUInt() == 42);
    SLANG_CHECK(r.ReadUInt() == 4000000000U);
}

SLANG_UNIT_TEST(tokenReaderFloatLiterals)
{
    TokenReader r("0.0 1.5 -2.25 1e3 -3.14");
    SLANG_CHECK(r.ReadDouble() == 0.0);
    SLANG_CHECK(r.ReadDouble() == 1.5);
    SLANG_CHECK(r.ReadDouble() == -2.25);
    SLANG_CHECK(r.ReadDouble() == 1e3);
    SLANG_CHECK(r.ReadFloat() == -3.14f);
}

SLANG_UNIT_TEST(tokenReaderStringLiteral)
{
    TokenReader r("\"hello\" \"two words\" \"\"");
    SLANG_CHECK(r.ReadStringLiteral() == "hello");
    SLANG_CHECK(r.ReadStringLiteral() == "two words");
    SLANG_CHECK(r.ReadStringLiteral() == "");
}

SLANG_UNIT_TEST(tokenReaderOperators)
{
    TokenReader r("a + b == c");
    auto t = r.ReadToken();
    SLANG_CHECK(t.Type == TokenType::Identifier && t.Content == "a");
    t = r.ReadToken();
    SLANG_CHECK(t.Type == TokenType::OpAdd);
    t = r.ReadToken();
    SLANG_CHECK(t.Type == TokenType::Identifier && t.Content == "b");
    t = r.ReadToken();
    SLANG_CHECK(t.Type == TokenType::OpEql);
    t = r.ReadToken();
    SLANG_CHECK(t.Type == TokenType::Identifier && t.Content == "c");
}

SLANG_UNIT_TEST(tokenReaderPunctuation)
{
    TokenReader r("(){}[];,.");
    SLANG_CHECK(r.ReadToken().Type == TokenType::LParent);
    SLANG_CHECK(r.ReadToken().Type == TokenType::RParent);
    SLANG_CHECK(r.ReadToken().Type == TokenType::LBrace);
    SLANG_CHECK(r.ReadToken().Type == TokenType::RBrace);
    SLANG_CHECK(r.ReadToken().Type == TokenType::LBracket);
    SLANG_CHECK(r.ReadToken().Type == TokenType::RBracket);
    SLANG_CHECK(r.ReadToken().Type == TokenType::Semicolon);
    SLANG_CHECK(r.ReadToken().Type == TokenType::Comma);
    SLANG_CHECK(r.ReadToken().Type == TokenType::Dot);
}

SLANG_UNIT_TEST(tokenReaderLookAheadAndAdvance)
{
    TokenReader r("foo bar baz");
    SLANG_CHECK(r.LookAhead(String("foo")));
    SLANG_CHECK(r.LookAhead(TokenType::Identifier));
    SLANG_CHECK(r.AdvanceIf(String("foo")));
    SLANG_CHECK(!r.AdvanceIf(String("not-here")));
    SLANG_CHECK(r.AdvanceIf(TokenType::Identifier)); // consumes "bar"
    SLANG_CHECK(r.ReadWord() == "baz");
    SLANG_CHECK(r.IsEnd());
}

SLANG_UNIT_TEST(tokenReaderReadExpected)
{
    TokenReader r("foo bar");
    SLANG_CHECK(r.Read("foo") == "foo");
    SLANG_CHECK(r.Read(String("bar")) == "bar");
}

SLANG_UNIT_TEST(tokenReaderReadType)
{
    TokenReader r("123 hello");
    SLANG_CHECK(r.Read(TokenType::IntLiteral));
    SLANG_CHECK(r.Read(TokenType::Identifier));
}

SLANG_UNIT_TEST(tokenReaderBack)
{
    TokenReader r("foo bar baz");
    SLANG_CHECK(r.ReadWord() == "foo");
    SLANG_CHECK(r.ReadWord() == "bar");
    r.Back(1);
    SLANG_CHECK(r.ReadWord() == "bar"); // re-read after Back
    SLANG_CHECK(r.ReadWord() == "baz");
}

SLANG_UNIT_TEST(tokenReaderTokenPosition)
{
    TokenReader r("foo\nbar");
    Token t = r.ReadToken();
    SLANG_CHECK(t.Type == TokenType::Identifier);
    int line1 = t.Position.Line;

    t = r.ReadToken();
    int line2 = t.Position.Line;

    // Whatever the convention, the second token must be on a later
    // line than the first when separated by a newline.
    SLANG_CHECK(line2 > line1);
}

SLANG_UNIT_TEST(tokenReaderReadIntThrowsOnNonInt)
{
    TokenReader r("not-a-number");
    bool threw = false;
    try
    {
        r.ReadInt();
    }
    catch (const TextFormatException&)
    {
        threw = true;
    }
    SLANG_CHECK(threw);
}

SLANG_UNIT_TEST(tokenReaderReadStringThrowsOnIdentifier)
{
    TokenReader r("foo");
    bool threw = false;
    try
    {
        r.ReadStringLiteral();
    }
    catch (const TextFormatException&)
    {
        threw = true;
    }
    SLANG_CHECK(threw);
}

SLANG_UNIT_TEST(tokenReaderReadAfterEnd)
{
    TokenReader r("foo");
    SLANG_CHECK(r.ReadWord() == "foo");
    bool threw = false;
    try
    {
        r.ReadToken();
    }
    catch (const TextFormatException&)
    {
        threw = true;
    }
    SLANG_CHECK(threw);
}
