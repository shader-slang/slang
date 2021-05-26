
#include "../../source/compiler-core/slang-json-lexer.h"
#include "../../source/core/slang-string-escape-util.h"

#include "test-context.h"

using namespace Slang;

namespace { // anonymous

struct Element
{
    JSONTokenType type;
    const char* value;
};

} // anonymous

static SlangResult _lex(const char* in, DiagnosticSink* sink, List<JSONToken>& toks)
{
    SourceManager* sourceManager = sink->getSourceManager();

    String contents(in);
    SourceFile* sourceFile = sourceManager->createSourceFileWithString(PathInfo::makeUnknown(), contents);
    SourceView* sourceView = sourceManager->createSourceView(sourceFile, nullptr, SourceLoc());

    JSONLexer lexer;

    lexer.init(sourceView, sink);

    while (lexer.peekType() != JSONTokenType::EndOfFile)
    {
        if (lexer.peekType() == JSONTokenType::Invalid)
        {
            toks.add(lexer.peekToken());
            return SLANG_FAIL;
        }

        toks.add(lexer.peekToken());
        lexer.advance();
    }

    toks.add(lexer.peekToken());

    // If we advance from end of file we should still be at EndOfFile
    SLANG_ASSERT(lexer.advance() == JSONTokenType::EndOfFile);

    return SLANG_OK;
}

static bool _areEqual(SourceManager* sourceManager, const List<JSONToken>& toks, const Element* eles, Index elesCount)
{
    if (toks.getCount() != elesCount)
    {
        return false;
    }

    SourceView* sourceView = toks.getCount() ? sourceManager->findSourceView(toks[0].loc) : nullptr;
    const char*const content = sourceView ? sourceView->getContent().begin() : nullptr;

    for (Index i = 0; i < toks.getCount(); ++i)
    {
        const JSONToken& tok = toks[i];
        const auto& ele = eles[i];

        if (tok.type != ele.type)
        {
            return false;
        }

        SLANG_ASSERT(sourceView->getRange().contains(tok.loc));

        const char* start = content + sourceView->getRange().getOffset(tok.loc);

        UnownedStringSlice lexeme(start, tok.length);

        if (lexeme != ele.value)
        {
            return false;
        }
    }

    return true;
}

static void jsonUnitTest()
{
    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);
    DiagnosticSink sink(&sourceManager, nullptr);

    {
        const char text[] = " { \"Hello\" : [ \"World\", 1, 2.0, -3.0, -435.5345435, 45e-10, 421.00e+20, 17e1] }";

        const Element eles[] =
        {
            {JSONTokenType::LBrace, "{" },
            {JSONTokenType::StringLiteral, "\"Hello\""},
            {JSONTokenType::Colon, ":" },
            {JSONTokenType::LBracket, "[" },
            {JSONTokenType::StringLiteral, "\"World\"" },
            {JSONTokenType::Comma, "," },
            {JSONTokenType::IntegerLiteral, "1" },
            {JSONTokenType::Comma, "," },
            {JSONTokenType::FloatLiteral, "2.0" },
            {JSONTokenType::Comma, "," },
            {JSONTokenType::FloatLiteral, "-3.0" },
            {JSONTokenType::Comma, "," },
            {JSONTokenType::FloatLiteral, "-435.5345435" },
            {JSONTokenType::Comma, "," },
            {JSONTokenType::FloatLiteral, "45e-10" },
            {JSONTokenType::Comma, "," },
            {JSONTokenType::FloatLiteral, "421.00e+20" },
            {JSONTokenType::Comma, "," },
            {JSONTokenType::FloatLiteral, "17e1" },
            {JSONTokenType::RBracket, "]" },
            {JSONTokenType::RBrace, "}" },
            {JSONTokenType::EndOfFile, "" },
        };

        List<JSONToken> toks;
        SLANG_CHECK(SLANG_SUCCEEDED(_lex(text, &sink, toks)));

        SLANG_CHECK(_areEqual(&sourceManager, toks, eles, SLANG_COUNT_OF(eles)));
    }

    {
        StringEscapeHandler* handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::JSON);

        
        {
            const auto slice = UnownedStringSlice::fromLiteral("\n\r\b\f\t \"\\/ Some text...");

            SLANG_CHECK(handler->isEscapingNeeded(slice));
            SLANG_CHECK(!handler->isEscapingNeeded(UnownedStringSlice::fromLiteral("Hello!")));

            StringBuilder escaped;
            handler->appendEscaped(slice, escaped);

            StringBuilder unescaped;
            handler->appendUnescaped(escaped.getUnownedSlice(), unescaped);

            SLANG_CHECK(unescaped == slice);
        }

        {
            uint32_t v = 0x7f;

            StringBuilder buf;
            while (v < 0x10000)
            {
                char work[10] = "\\u";

                for (Int i = 0; i < 4; ++i)
                {
                    const uint32_t digitValue = (v >> ((3 - i) * 4)) & 0xf;

                    char digitC = (digitValue > 9) ? char(digitValue - 10 + 'a') : char(digitValue + '0');
                    work[i + 2] =  digitC;
                }

                buf << UnownedStringSlice(work, 6);

                v += v;
            }

            // Decode it
            StringBuilder unescaped;
            handler->appendUnescaped(buf.getUnownedSlice(), unescaped);

            // Encode it
            StringBuilder escaped;
            handler->appendEscaped(unescaped.getUnownedSlice(), escaped);

            SLANG_CHECK(escaped == buf);
        }
    }
}

SLANG_UNIT_TEST("JSON", jsonUnitTest);
