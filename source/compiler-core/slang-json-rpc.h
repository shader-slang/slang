#ifndef SLANG_COMPILER_CORE_JSON_RPC_H
#define SLANG_COMPILER_CORE_JSON_RPC_H

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "slang-json-value.h"

#include "slang-json-parser.h"
#include "../core/slang-http.h"

namespace Slang {

/// Send and receive messages as JSON
class JSONRPCUtil
{
public:
    
        /// Parse slice into JSONContainer. outValue is the root of the hierarchy.
    static SlangResult parseJSON(const UnownedStringSlice& slice, JSONContainer* container, DiagnosticSink* sink, JSONValue& outValue);

        /// Parse content from stream, and consume the packet
    static SlangResult parseJSONAndConsume(HTTPPacketConnection* connection, JSONContainer* container, DiagnosticSink* sink, JSONValue& outValue);
};

} // namespace Slang

#endif // SLANG_COMPILER_CORE_JSON_RPC_H
