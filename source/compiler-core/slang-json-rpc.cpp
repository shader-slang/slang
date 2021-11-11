#include "slang-json-rpc.h"

namespace Slang {

JSONConnection::JSONConnection(HTTPPacketConnection* connection) :
    m_connection(connection)
{
    m_sourceManager.initialize(nullptr, nullptr);
    m_diagnosticSink.init(&m_sourceManager, &JSONLexer::calcLexemeLocation);
}

SlangResult JSONConnection::parseJSON(const UnownedStringSlice& slice, JSONContainer* container, JSONValue& outValue)
{
    // Now need to parse as JSON
    String contents(slice);
    SourceFile* sourceFile = m_sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), contents);
    SourceView* sourceView = m_sourceManager.createSourceView(sourceFile, nullptr, SourceLoc());

    JSONLexer lexer;
    lexer.init(sourceView, &m_diagnosticSink);

    JSONBuilder builder(container);

    JSONParser parser;
    SLANG_RETURN_ON_FAIL(parser.parse(&lexer, sourceView, &builder, &m_diagnosticSink));

    outValue = builder.getRootValue();
    return SLANG_OK;
}

SlangResult JSONConnection::readJSONContent(JSONContainer* container, JSONValue& outValue)
{
    if (!hasContent())
    {
        return SLANG_FAIL;
    }

    auto content = m_connection->getContent();

    UnownedStringSlice text((const char*)content.begin(), content.getCount());
    SlangResult res = parseJSON(text, container, outValue);

    m_connection->consumeContent();
    return res;
}

SlangResult JSONConnection::waitForReadJSONContent(JSONContainer* container, JSONValue& outValue)
{
    SLANG_RETURN_ON_FAIL(m_connection->waitForContent());

    // If we don't actually have content we could be at the end
    if (!m_connection->hasContent())
    {
        outValue = JSONValue::makeInvalid();
        return SLANG_OK;
    }

    SLANG_RETURN_ON_FAIL(readJSONContent(container, outValue));
    return SLANG_OK;
}

} // namespace Slang
