// slang-json-lexer.h
#ifndef SLANG_JSON_LEXER_H
#define SLANG_JSON_LEXER_H

#include "../core/slang-basic.h"

#include "slang-source-loc.h"
#include "slang-diagnostic-sink.h"

namespace Slang {

enum class JSONTokenType
{
    Invalid,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    LBracket,
    RBracket,
    LBrace,
    RBrace,
    Comma,
    Colon,
    True,
    False,
    Null,
    EndOfFile,
    CountOf,
};

struct JSONToken
{
    JSONTokenType type;         ///< The token type 
    SourceLoc loc;              ///< Location in the source file
    uint32_t length;            ///< The length of the token in bytes
};

class JSONLexer
{
public:
    JSONToken& peekToken() { return m_token; }
    JSONTokenType peekType() { return m_token.type; }

    JSONTokenType advance();

    SlangResult init(SourceView* sourceView, DiagnosticSink* sink);

protected:
    struct LexResult
    {
        JSONTokenType type;
        const char* cursor;
    };

        /// Get the location of the cursor
    SLANG_FORCE_INLINE SourceLoc _getLoc(const char* cursor) const { return m_startLoc + (cursor - m_contentStart); }
    const char* _lexLineComment(const char* cursor);
    const char* _lexBlockComment(const char* cursor);
    const char* _lexWhitespace(const char* cursor);
    const char* _lexString(const char* cursor);
    LexResult _lexNumber(const char* cursor);

    SLANG_FORCE_INLINE JSONTokenType _setToken(JSONTokenType type, const char* cursor)
    {
        SLANG_ASSERT(cursor >= m_lexemeStart);
        m_token.type = type;
        m_token.loc = m_startLoc + (m_lexemeStart - m_contentStart);
        m_token.length = uint32_t(cursor - m_lexemeStart);
        m_cursor = cursor;
        return type;
    }
    JSONTokenType _setInvalidToken();

    JSONToken m_token;

    const char* m_cursor;
    const char* m_lexemeStart;

    const char* m_contentStart;

    SourceLoc m_startLoc;

    SourceView* m_sourceView;
    DiagnosticSink* m_sink;
};

} // namespace Slang

#endif
