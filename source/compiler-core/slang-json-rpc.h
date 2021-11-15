#ifndef SLANG_COMPILER_CORE_JSON_RPC_H
#define SLANG_COMPILER_CORE_JSON_RPC_H

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "slang-json-value.h"

#include "slang-json-parser.h"
#include "../core/slang-http.h"

namespace Slang {

struct JSONRPCErrorResponse
{
    struct Error
    {
        Index code = 0;                     ///< Value from ErrorCode
        UnownedStringSlice message;         ///< Error message

        static const RttiInfo* createRttiInfo();
    };

    Error error;
    JSONValue data;                 
    Int id = -1;                        ///< Id of initiating method or -1 if not set

    static const RttiInfo* createRttiInfo();
};

SLANG_STRUCT_RTTI_INFO(JSONRPCErrorResponse::Error)
SLANG_STRUCT_RTTI_INFO(JSONRPCErrorResponse)

struct JSONRPCCall
{
    UnownedStringSlice method;          ///< The name of the method
    JSONValue params;                   ///< Can be invalid/array/object
    Int id = -1;                        ///< Id associated with this request, or -1 if not set

    static const RttiInfo* createRttiInfo();
};

SLANG_STRUCT_RTTI_INFO(JSONRPCCall)

struct JSONResultResponse
{
    JSONValue result;                   ///< The result value
    Int id = -1;                        ///< Id of initiating method or -1 if not set

    static const RttiInfo* createRttiInfo();
};

SLANG_STRUCT_RTTI_INFO(JSONResultResponse)

/// Send and receive messages as JSON
///
/// Strictly speaking should support ids, as strings or ids. Currently just supports with integer ids.
/// One way of dealing with this would be to just use JSONValue for ids, would allow invalid/string/integer and
/// a mechanism to compare/display etc.
class JSONRPCUtil
{
public:

    enum class ErrorCode
    {
        ParseError = -32700,        ///< Invalid JSON was received by the server.
        InvalidRequest = -32600,    ///< The JSON sent is not a valid Request object.
        MethodNotFound = -32601,    ///< The method does not exist / is not available.
        InvalidParams = -32602,     ///< Invalid method parameter(s).
        InternalError = -32603, 	///< Internal JSON - RPC error.

        ServerImplStart = -32000,   ///< Server implementation defined error range
        ServerImplEnd = -32099,
    };

    enum class ResponseType
    {
        Invalid,
        Error,
        Result
    };

    struct ErrorResponse
    {
        Index code = 0;                     ///< Value from ErrorCode
        UnownedStringSlice message;         ///< Error message
        JSONValue data;
        Int id = -1;                        ///< Id of initiating method or -1 if not set
    };

    struct ResultResponse
    {
        JSONValue result;                   ///< The result value
        Int id = -1;                        ///< Id of initiating method or -1 if not set
    };

    struct Call
    {
        UnownedStringSlice method;          ///< The name of the method
        JSONValue params;                   ///< Can be invalid/array/object
        Int id = -1;                        ///< Id associated with this request, or -1 if not set
    };

        /// Parameters can be either named or via index.
    static JSONValue createCall(JSONContainer* container, const UnownedStringSlice& method, JSONValue params, Int id = -1);
        /// Parameters can be either named or via index.
    static JSONValue createCall(JSONContainer* container, const UnownedStringSlice& method, Int id = -1);

        /// Create an error response
        /// Code should typically be something in the ErrorCode range
    static JSONValue createErrorResponse(JSONContainer* container, Index code, const UnownedStringSlice& message, const JSONValue& data = JSONValue(), Int id = -1);
    static JSONValue createErrorResponse(JSONContainer* container, ErrorCode code, const UnownedStringSlice& message, const JSONValue& data = JSONValue(), Int id = -1);
        /// Create a result response
    static JSONValue createResultResponse(JSONContainer* container, const JSONValue& resultValue, Int id = -1);

        /// Determine the response type
    static ResponseType getResponseType(JSONContainer* container, const JSONValue& response);

    static SlangResult parseError(JSONContainer* container, const JSONValue& response, ErrorResponse& out);

    static SlangResult parseResult(JSONContainer* container, const JSONValue& response, ResultResponse& out);

    static SlangResult parseCall(JSONContainer* container, const JSONValue& value, Call& out);

        /// Parse slice into JSONContainer. outValue is the root of the hierarchy.
    static SlangResult parseJSON(const UnownedStringSlice& slice, JSONContainer* container, DiagnosticSink* sink, JSONValue& outValue);

        /// Parse content from stream, and consume the packet
    static SlangResult parseJSONAndConsume(HTTPPacketConnection* connection, JSONContainer* container, DiagnosticSink* sink, JSONValue& outValue);
};

} // namespace Slang

#endif // SLANG_COMPILER_CORE_JSON_RPC_H
