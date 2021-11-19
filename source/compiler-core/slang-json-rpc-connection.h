#ifndef SLANG_COMPILER_CORE_JSON_RPC_CONNECTION_H
#define SLANG_COMPILER_CORE_JSON_RPC_CONNECTION_H

#include "../../source/core/slang-http.h"
#include "../../source/core/slang-process.h"

#include "slang-diagnostic-sink.h"
#include "slang-source-loc.h"
#include "slang-json-value.h"
#include "slang-json-rpc.h"

#include "slang-test-server-protocol.h"

namespace Slang {

/* TODO(JS):

It's not really clear here, how to handle id handling. If a server can receive multiple messages, and then later send responses we need
a way to queue up ids. This is complicated by JSONValue is only valid whilst the backing JSONContainer holds it.

We probably want to create some kind of Variant that can hold all of these values that can hold state independently.
*/
class JSONRPCConnection : public RefObject
{
public:

        /// An init function must be called before use
        /// If a process is implementing the server it should be passed in if the process needs to shut down if the connection does
    SlangResult init(HTTPPacketConnection* connection, Process* process = nullptr);

        /// Initialize using stdin/out streams for input/output. 
    SlangResult initWithStdStreams(Process* process = nullptr);

        /// Disconnect. May block while server shuts down
    void disconnect();

        /// Convert value to dst. Will write response on fails
    SlangResult toNativeOrSendError(const JSONValue& value, const RttiInfo* info, void* dst);
    template <typename T>
    SlangResult toNativeOrSendError(const JSONValue& value, T* data) { return toNativeOrSendError(value, GetRttiInfo<T>::get(), data); }

    template <typename T>
    SlangResult toValidNativeOrSendError(const JSONValue& value, T* data);

        /// Send a RPC response (ie should only be one of the JSONRPC classes)
    SlangResult sendRPC(const RttiInfo* info, const void* data);
    template <typename T>
    SlangResult sendRPC(const T* data) { return sendRPC(GetRttiInfo<T>::get(), (const void*)data);  }

        /// Send an error
    SlangResult sendError(JSONRPC::ErrorCode code);
    SlangResult sendError(JSONRPC::ErrorCode errorCode, const UnownedStringSlice& msg);

        /// Send a call
        /// If no id is needed, id can just be invalid 
    SlangResult sendCall(const UnownedStringSlice& method, const RttiInfo* argsRttiInfo, const void* args, const JSONValue& id = JSONValue());
    template <typename T>
    SlangResult sendCall(const UnownedStringSlice& method, const T* args, const JSONValue& id = JSONValue()) { return sendCall(method, GetRttiInfo<T>::get(), (const void*)args, id); }
        ///
    SlangResult sendCall(const UnownedStringSlice& method, const JSONValue& id = JSONValue());

    template <typename T>
    SlangResult sendResult(const T* result, const JSONValue& id = JSONValue()) { return sendResult(GetRttiInfo<T>::get(), (const void*)result, id); }
    SlangResult sendResult(const RttiInfo* rttiInfo, const void* result, const JSONValue& id = JSONValue());

        /// Try to read a message. Will return if a message is not available.
    SlangResult tryReadMessage();

        /// Will block waiting for a message.
    SlangResult waitForResult();

        /// If we have an JSON-RPC message m_jsonRoot the root.
    bool hasMessage() const { return m_jsonRoot.isValid(); }

        /// If there is a message returns kind of JSON RPC message
    JSONRPCMessageType getMessageType();

        /// Get JSON-RPC message (ie one of JSONRPC classes)
    template <typename T>
    SlangResult getMessage(T* out) { return getMessage(GetRttiInfo<T>::get(), (void*)out); }
    SlangResult getMessage(const RttiInfo* rttiInfo, void* out);

        /// Get JSON-RPC message (ie one of JSONRPC prefixed classes)
        /// If there is a message and there is a failure, will send an error response
    template <typename T>
    SlangResult getMessageOrSendError(T* out) { return getMessageOrSendError(GetRttiInfo<T>::get(), (void*)out); }
    SlangResult getMessageOrSendError(const RttiInfo* rttiInfo, void* out);


        /// Clears all the internal buffers (for JSON/Source/etc).
        /// Happens automatically on tryReadMessage/readMessage
    void clearBuffers();

        /// True if this connection is active
    bool isActive();

        /// Get the id of the current message
    JSONValue getMessageId();

        /// Get the diagnostic sink. Can queue up errors before sending an error
    DiagnosticSink* getSink() { return &m_diagnosticSink;  }

        /// Dtor
    ~JSONRPCConnection() { disconnect(); }

        /// Ctor
    JSONRPCConnection():m_container(nullptr) {}

protected:
    RefPtr<Slang::Process> m_process;                       ///< Backing process (optional)
    RefPtr<Slang::HTTPPacketConnection> m_connection;

    DiagnosticSink m_diagnosticSink;

    SourceManager m_sourceManager;
    JSONContainer m_container;

    JSONValue m_jsonRoot;

        /// Default timeout is 10 seconds
    Int m_timeOutInMs = 10 * 1000;
        /// Termination timeout
    Int m_terminationTimeOutInMs = 1 * 1000;
};

// ---------------------------------------------------------------------------
template <typename T>
SlangResult JSONRPCConnection::toValidNativeOrSendError(const JSONValue& value, T* data)
{
    const RttiInfo* rttiInfo = GetRttiInfo<T>::get();

    SLANG_RETURN_ON_FAIL(toNativeOrRespond(value, rttiInfo, (void*)data));
    if (!data->isValid())
    {
        // If it has a name add validation info
        if (rttiInfo->isNamed())
        {
            const NamedRttiInfo* namedRttiInfo = static_cast<const NamedRttiInfo*>(rttiInfo);
            m_diagnosticSink.diagnose(SourceLoc(), ServerDiagnostics::argsAreInvalid, namedRttiInfo->m_name);
        }

        return respondWithError(JSONRPC::ErrorCode::InvalidRequest);
    }
    return SLANG_OK;
}

} // namespace Slang

#endif // SLANG_COMPILER_CORE_JSON_RPC_CONNECTION_H

