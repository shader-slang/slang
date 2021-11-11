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
class JSONConnection : public RefObject
{
public:

    bool hasContent() const { return m_connection->hasContent();  }

        /// Only valid if has content
    SlangResult readJSONContent(JSONContainer* container, JSONValue& outValue);
        /// Will wait for content
    SlangResult waitForReadJSONContent(JSONContainer* container, JSONValue& outValue);

        /// Parse slice into JSONContainer. outValue is the root of the hierarchy.
    SlangResult parseJSON(const UnownedStringSlice& slice, JSONContainer* container, JSONValue& outValue);

        /// Get the diagnostic sink
    DiagnosticSink* getDiagnosticSink() { return &m_diagnosticSink;  }
        /// Get the underlying protocol connection
    HTTPPacketConnection* getConnection() { return m_connection;  }
        /// Get the source manager
    SourceManager* getSourceManager() { return &m_sourceManager;  }

        /// Ctor
    JSONConnection(HTTPPacketConnection* connection);

protected:

    RefPtr<HTTPPacketConnection> m_connection;

    SourceManager m_sourceManager;
    DiagnosticSink m_diagnosticSink;
};

} // namespace Slang

#endif // SLANG_COMPILER_CORE_JSON_RPC_H
