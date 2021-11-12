#include "slang-json-rpc.h"

namespace Slang {

// https://www.jsonrpc.org/specification

//    m_sourceManager.initialize(nullptr, nullptr);
//    m_diagnosticSink.init(&m_sourceManager, &JSONLexer::calcLexemeLocation);

/* static */SlangResult JSONRPCUtil::parseJSON(const UnownedStringSlice& slice, JSONContainer* container, DiagnosticSink* sink, JSONValue& outValue)
{
    SourceManager* sourceManager = sink->getSourceManager();

    // Now need to parse as JSON
    String contents(slice);
    SourceFile* sourceFile = sourceManager->createSourceFileWithString(PathInfo::makeUnknown(), contents);
    SourceView* sourceView = sourceManager->createSourceView(sourceFile, nullptr, SourceLoc());

    JSONLexer lexer;
    lexer.init(sourceView, sink);

    JSONBuilder builder(container);

    JSONParser parser;
    SLANG_RETURN_ON_FAIL(parser.parse(&lexer, sourceView, &builder, sink));

    outValue = builder.getRootValue();
    return SLANG_OK;
}

SlangResult JSONRPCUtil::parseJSONAndConsume(HTTPPacketConnection* connection, JSONContainer* container, DiagnosticSink* sink, JSONValue& outValue)
{
    if (!connection->hasContent())
    {
        return SLANG_FAIL;
    }

    auto content = connection->getContent();

    UnownedStringSlice text((const char*)content.begin(), content.getCount());
    SlangResult res = parseJSON(text, container, sink, outValue);

    // Consume the content
    connection->consumeContent();
    return res;
}

} // namespace Slang
