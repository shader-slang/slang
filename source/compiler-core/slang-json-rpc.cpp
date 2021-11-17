#include "slang-json-rpc.h"

#include "../../slang-com-helper.h"

namespace Slang {

// https://www.jsonrpc.org/specification

//    m_sourceManager.initialize(nullptr, nullptr);
//    m_diagnosticSink.init(&m_sourceManager, &JSONLexer::calcLexemeLocation);

static const auto g_jsonRpc = UnownedStringSlice::fromLiteral("jsonrpc");
static const auto g_jsonRpcVersion = UnownedStringSlice::fromLiteral("2.0");
static const auto g_method = UnownedStringSlice::fromLiteral("method");
static const auto g_id = UnownedStringSlice::fromLiteral("id");
static const auto g_params = UnownedStringSlice::fromLiteral("params");
static const auto g_code = UnownedStringSlice::fromLiteral("code");
static const auto g_error = UnownedStringSlice::fromLiteral("error");
static const auto g_message = UnownedStringSlice::fromLiteral("message");
static const auto g_result = UnownedStringSlice::fromLiteral("result");
static const auto g_data = UnownedStringSlice::fromLiteral("data");

// Add the fields.
// TODO(JS): This is a little verbose, and could be improved on with something like
// * Tool that automatically generated from C++ (say via the C++ extractor)
// * Macro magic to simplify the construction
static const StructRttiInfo _makeJSONRPCErrorResponse_Error()
{
    JSONRPCErrorResponse::Error obj;
    StructRttiBuilder builder(&obj, "JSONRPCErrorResponse::Error", nullptr);
    builder.addField("code", &obj.code);
    builder.addField("message", &obj.message);
    return builder.make();
}
/* static */const StructRttiInfo JSONRPCErrorResponse::Error::g_rttiInfo = _makeJSONRPCErrorResponse_Error();

static const StructRttiInfo _makeJSONRPCErrorResponse()
{
    JSONRPCErrorResponse obj;
    StructRttiBuilder builder(&obj, "JSONRPCErrorResponse", nullptr);

    builder.addField("error", &obj.error);
    builder.addField("data", &obj.data, StructRttiInfo::Flag::Optional);
    builder.addField("id", &obj.id, combine(StructRttiInfo::Flag::Optional, RttiDefaultValue::MinusOne));

    return builder.make();
}
/* static */const StructRttiInfo JSONRPCErrorResponse::g_rttiInfo = _makeJSONRPCErrorResponse();

static const StructRttiInfo _makeJSONRPCCallResponse()
{
    JSONRPCCall obj;
    StructRttiBuilder builder(&obj, "JSONRPCCall", nullptr);

    builder.addField("method", &obj.method);
    builder.addField("params", &obj.params, StructRttiInfo::Flag::Optional);
    builder.addField("id", &obj.id, combine(StructRttiInfo::Flag::Optional, RttiDefaultValue::MinusOne));

    return builder.make();
}
/* static */const StructRttiInfo JSONRPCCall::g_rttiInfo = _makeJSONRPCCallResponse();

static const StructRttiInfo _makeJSONResultResponseResponse()
{
    JSONResultResponse obj;
    StructRttiBuilder builder(&obj, "JSONResultResponse", nullptr);

    builder.addField("result", &obj.result);
    builder.addField("id", &obj.id, combine(StructRttiInfo::Flag::Optional, RttiDefaultValue::MinusOne));

    return builder.make();
}
/* static */const StructRttiInfo JSONResultResponse::g_rttiInfo = _makeJSONResultResponseResponse();




/* static */JSONValue JSONRPCUtil::createCall(JSONContainer* container, const UnownedStringSlice& method, JSONValue params, Int id)
{
    const Index maxPairs = 4;
    JSONKeyValue pairs[maxPairs];

    Index i = 0;

    // Version number is a string
    pairs[i++] = JSONKeyValue::make(container->getKey(g_jsonRpc), container->createString(g_jsonRpcVersion));
    pairs[i++] = JSONKeyValue::make(container->getKey(g_method), container->createString(method));
    pairs[i++] = JSONKeyValue::make(container->getKey(g_params), params);

    if (id >= 0)
    {
        pairs[i++] = JSONKeyValue::make(container->getKey(g_id), JSONValue::makeInt(id));
    }

    return container->createObject(pairs, i);
}

/* static */JSONValue JSONRPCUtil::createCall(JSONContainer* container, const UnownedStringSlice& method, Int id)
{
    const Index maxPairs = 3;
    JSONKeyValue pairs[maxPairs];
    Index i = 0;
    // Version number is a string
    pairs[i++] = JSONKeyValue::make(container->getKey(g_jsonRpc), container->createString(g_jsonRpcVersion));
    pairs[i++] = JSONKeyValue::make(container->getKey(g_method), container->createString(method));

    if (id >= 0)
    {
        pairs[i++] = JSONKeyValue::make(container->getKey(g_id), JSONValue::makeInt(id));
    }

    return container->createObject(pairs, i);
}

/* static */JSONValue JSONRPCUtil::createErrorResponse(JSONContainer* container, Index code, const UnownedStringSlice& message, const JSONValue& data, Int id)
{
    // Set up the error value
    JSONValue errorValue;
    {
        const Index maxPairs = 2;
        JSONKeyValue pairs[maxPairs];
        Index i = 0;

        if (code != 0)
        {
            pairs[i++] = JSONKeyValue::make(container->getKey(g_code), JSONValue::makeInt(code));
        }
        if (message.getLength() > 0)
        {
            pairs[i++] = JSONKeyValue::make(container->getKey(g_message), container->createString(message));
        }
        errorValue = container->createObject(pairs, i);
    }

    const Index maxPairs = 4;
    JSONKeyValue pairs[maxPairs];
    Index i = 0;

    pairs[i++] = JSONKeyValue::make(container->getKey(g_jsonRpc), container->createString(g_jsonRpcVersion));
    pairs[i++] = JSONKeyValue::make(container->getKey(g_error), errorValue);

    if (data.isValid())
    {
        pairs[i++] = JSONKeyValue::make(container->getKey(g_data), data);
    }

    if (id >= 0)
    {
        pairs[i++] = JSONKeyValue::make(container->getKey(g_id), JSONValue::makeInt(id));
    }

    return container->createObject(pairs, i);
}


/* static */JSONValue JSONRPCUtil::createErrorResponse(JSONContainer* container, ErrorCode code, const UnownedStringSlice& message, const JSONValue& data, Int id)
{
    return createErrorResponse(container, Index(code), message, data, id);
}

/* static */JSONValue JSONRPCUtil::createResultResponse(JSONContainer* container, const JSONValue& resultValue, Int id)
{
    const Index maxPairs = 3;
    JSONKeyValue pairs[maxPairs];
    Index i = 0;

    pairs[i++] = JSONKeyValue::make(container->getKey(g_jsonRpc), container->createString(g_jsonRpcVersion));
    pairs[i++] = JSONKeyValue::make(container->getKey(g_result), resultValue);

    if (id >= 0)
    {
        pairs[i++] = JSONKeyValue::make(container->getKey(g_id), JSONValue::makeInt(id));
    }

    return container->createObject(pairs, i);
}

/* static */ JSONRPCUtil::ResponseType JSONRPCUtil::getResponseType(JSONContainer* container, const JSONValue& response)
{
    if (response.getKind() == JSONValue::Kind::Object)
    {
        const JSONKey resultKey = container->findKey(g_result);
        const JSONKey errorKey = container->findKey(g_error);

        auto pairs = container->getObject(response);

        for (const auto& pair : pairs)
        {
            if (pair.key == resultKey)
            {
                return ResponseType::Result;
            }
            else if (pair.key == errorKey)
            {
                return ResponseType::Error;
            }
        }
    }

    return ResponseType::Error;
}

static SlangResult _parseError(JSONContainer* container, const JSONValue& error, JSONRPCUtil::ErrorResponse& out)
{
    if (error.getKind() != JSONValue::Kind::Object)
    {
        return SLANG_FAIL;
    }
    const auto pairs = container->getObject(error);

    const JSONKey messageKey = container->findKey(g_message);
    const JSONKey codeKey = container->findKey(g_code);
    const JSONKey dataKey = container->findKey(g_data);

    Int fieldBits = 0;

    for (auto const& pair : pairs)
    {
        if (pair.key == messageKey)
        {
            if (pair.value.getKind() != JSONValue::Kind::String)
            {
                return SLANG_FAIL;
            }
            out.message = container->getString(pair.value);
            fieldBits |= 0x1;
        }
        else if (pair.key == codeKey)
        {
            if (pair.value.getKind() != JSONValue::Kind::Integer)
            {
                return SLANG_FAIL;
            }
            out.code = Index(pair.value.asInteger());
            fieldBits |= 0x2;
        }
        else if (pair.key == dataKey)
        {
            out.data = pair.value;
            fieldBits |= 0x4;
        }
        else
        {
            return SLANG_FAIL;
        }
    }

    // Check all required fields are set
    return (fieldBits & 0x3) == 0x3 ? SLANG_OK : SLANG_FAIL;
}

/* static */SlangResult JSONRPCUtil::parseError(JSONContainer* container, const JSONValue& response, ErrorResponse& out)
{
    if (response.getKind() != JSONValue::Kind::Object)
    {
        return SLANG_FAIL;
    }

    const auto pairs = container->getObject(response);

    const JSONKey jsonRpcKey = container->findKey(g_jsonRpc);
    const JSONKey errorKey = container->findKey(g_error);
    const JSONKey idKey = container->findKey(g_id);

    Int fieldBits = 0;

    for (auto const& pair : pairs)
    {
        if (pair.key == jsonRpcKey)
        {
            if (!container->areEqual(pair.value, g_jsonRpcVersion))
            {
                return SLANG_FAIL;
            }
            fieldBits |= 0x1;
        }
        else if (pair.key == errorKey)
        {
            // We need to decode the error 
            SLANG_RETURN_ON_FAIL(_parseError(container, pair.value, out));
            fieldBits |= 0x2;
        }
        else if (pair.key == idKey)
        {
            if (pair.value.getKind() != JSONValue::Kind::Integer)
            {
                return SLANG_FAIL;
            }
            out.id = pair.value.asInteger();
            fieldBits |= 0x4;
        }
        else
        {
            // Unknown key
            return SLANG_FAIL;
        }
    }

    // Check all the required bits are set
    return ((fieldBits & 0x3) == 0x3) ? SLANG_OK : SLANG_FAIL;
}

/* static */SlangResult JSONRPCUtil::parseResult(JSONContainer* container, const JSONValue& response, ResultResponse& out)
{
    if (response.getKind() != JSONValue::Kind::Object)
    {
        return SLANG_FAIL;
    }

    const auto pairs = container->getObject(response);

    const JSONKey jsonRpcKey = container->findKey(g_jsonRpc);
    const JSONKey resultKey = container->findKey(g_result);
    const JSONKey idKey = container->findKey(g_id);

    Int fieldBits = 0;

    for (auto const& pair : pairs)
    {
        if (pair.key == jsonRpcKey)
        {
            if (!container->areEqual(pair.value, g_jsonRpcVersion))
            {
                return SLANG_FAIL;
            }
            fieldBits |= 0x1;
        }
        else if (pair.key == resultKey)
        {
            out.result = pair.value;
            fieldBits |= 0x2;
        }
        else if (pair.key == idKey)
        {
            if (pair.value.getKind() != JSONValue::Kind::Integer)
            {
                return SLANG_FAIL;
            }
            out.id = pair.value.asInteger();
            fieldBits |= 0x4;
        }
        else
        {
            // Unknown key
            return SLANG_FAIL;
        }
    }

    // Check all the required bits are set
    return ((fieldBits & 0x3) == 0x3) ? SLANG_OK : SLANG_FAIL;
}

/* static */SlangResult JSONRPCUtil::parseCall(JSONContainer* container, const JSONValue& value, Call& out)
{
    if (value.getKind() != JSONValue::Kind::Object)
    {
        return SLANG_FAIL;
    }

    const auto pairs = container->getObject(value);

    const JSONKey jsonRpcKey = container->findKey(g_jsonRpc);
    const JSONKey methodKey = container->findKey(g_method);
    const JSONKey paramsKey = container->findKey(g_params);
    const JSONKey idKey = container->findKey(g_id);

    Int fieldBits = 0;

    for (auto const& pair : pairs)
    {
        if (pair.key == jsonRpcKey)
        {
            if (!container->areEqual(pair.value, g_jsonRpcVersion))
            {
                return SLANG_FAIL;
            }
            fieldBits |= 0x1;
        }
        else if (pair.key == methodKey)
        {
            if (pair.value.getKind() != JSONValue::Kind::String)
            {
                return SLANG_FAIL;
            }
            out.method = container->getString(pair.value);
            fieldBits |= 0x2;
        }
        else if (pair.key == idKey)
        {
            if (pair.value.getKind() != JSONValue::Kind::Integer)
            {
                return SLANG_FAIL;
            }
            out.id = pair.value.asInteger();
            fieldBits |= 0x4;
        }
        else if (pair.key == paramsKey)
        {
            out.params = pair.value;
            fieldBits |= 0x8;
        }
        else
        {
            // Unknown key
            return SLANG_FAIL;
        }
    }

    // Check all the required bits are set
    return ((fieldBits & 0x3) == 0x3) ? SLANG_OK : SLANG_FAIL;
}


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
