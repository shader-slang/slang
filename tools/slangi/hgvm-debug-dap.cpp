#include "hgvm-debug-dap.h"

#include "compiler-core/slang-json-rpc.h"
#include "core/slang-blob.h"
#include "core/slang-http.h"
#include "core/slang-io.h"
#include "core/slang-process.h"
#include "hgbc-compiler.h"
#include "hgvm-native-host.h"
#include "slang-hgvm/slang-hgvm-image-io.h"
#include "slangi-options.h"

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>

namespace SlangITool
{
using namespace Slang;
using namespace Slang::HGVM;

namespace
{
/// Owns a message's JSON text and values. DAP uses HTTP framing but not JSON-RPC envelopes.
struct DAPMessage
{
    SourceManager sources;
    DiagnosticSink diagnostics;
    JSONContainer json;
    JSONValue root = JSONValue::makeEmptyObject();

    DAPMessage()
        : json(&sources)
    {
        sources.initialize(nullptr, nullptr);
        diagnostics.init(&sources, &JSONLexer::calcLexemeLocation);
    }
    bool parse(const String& text)
    {
        return SLANG_SUCCEEDED(
                   JSONRPCUtil::parseJSON(text.getUnownedSlice(), &json, &diagnostics, root)) &&
               root.getKind() == JSONValue::Kind::Object;
    }
    JSONValue get(JSONValue object, const char* key)
    {
        return object.getKind() == JSONValue::Kind::Object
                   ? json.findObjectValue(object, json.getKey(UnownedStringSlice(key)))
                   : JSONValue();
    }
    String string(JSONValue object, const char* key, const char* fallback = "")
    {
        auto value = get(object, key);
        return value.getKind() == JSONValue::Kind::String ? String(json.getString(value))
                                                          : String(fallback);
    }
    int64_t integer(JSONValue object, const char* key, int64_t fallback = 0)
    {
        auto value = get(object, key);
        return value.getKind() == JSONValue::Kind::Integer ? json.asInteger(value) : fallback;
    }
    bool boolean(JSONValue object, const char* key, bool fallback = false)
    {
        auto value = get(object, key);
        return value.getKind() == JSONValue::Kind::Bool ? json.asBool(value) : fallback;
    }
    void put(JSONValue& object, const char* key, JSONValue value)
    {
        json.setKeyValue(object, json.getKey(UnownedStringSlice(key)), value);
    }
    void put(JSONValue& object, const char* key, const String& value)
    {
        put(object, key, json.createString(value.getUnownedSlice()));
    }
    void number(JSONValue& object, const char* key, int64_t value)
    {
        put(object, key, JSONValue::makeInt(value));
    }
    void flag(JSONValue& object, const char* key, bool value)
    {
        put(object, key, JSONValue::makeBool(value));
    }
    String serialize(JSONValue value)
    {
        JSONWriter writer(JSONWriter::IndentationStyle::KNR);
        json.traverseRecursively(value, &writer);
        return writer.getBuilder().produceString();
    }
};

/// Only the reader touches the read connection. Session access and message writes stay on the
/// main thread, so native windows keep their thread affinity and inspection cannot race execution.
class DAPAdapter : public IDebugSessionListener
{
    struct OutputSink : IHGVMOutputSink
    {
        DAPAdapter* adapter;
        explicit OutputSink(DAPAdapter* adapter)
            : adapter(adapter)
        {
        }
        void writeLine(std::string_view text) override
        {
            adapter->output(
                String(UnownedStringSlice(text.data(), Index(text.size()))) + "\n",
                "stdout");
        }
    };
    struct VariableReference
    {
        Index frame = -1;
        bool arguments = false;
        uint64_t value = 0;
        uint32_t named = 0;
        uint32_t indexed = 0;
    };

    RefPtr<HTTPPacketConnection> m_input;
    RefPtr<HTTPPacketConnection> m_output;
    std::thread m_reader;
    std::mutex m_mutex;
    std::condition_variable m_ready;
    List<String> m_requests;
    DebugSession* m_session = nullptr; // Published to the reader only while holding m_mutex.
    bool m_closed = false;
    bool m_shutdown = false;
    bool m_initialized = false;
    bool m_launched = false;
    bool m_configured = false;
    bool m_running = false;
    bool m_finished = false;
    bool m_disconnected = false;
    bool m_stopOnEntry = false;
    bool m_linesStartAtOne = true;
    bool m_columnsStartAtOne = true;
    int64_t m_sequence = 1;
    int32_t m_nextHandle = 1;
    Dictionary<int32_t, Index> m_frames;
    List<int32_t> m_frameIds;
    Dictionary<int32_t, VariableReference> m_variables;
    RefPtr<NativeHGVMRuntime> m_runtime;
    String m_entry;

    void readRequests()
    {
        for (;;)
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_shutdown)
                    break;
            }
            m_input->waitForResult(20);
            if (m_input->hasContent())
            {
                auto bytes = m_input->getContent();
                String text(UnownedStringSlice((const char*)bytes.getBuffer(), bytes.getCount()));
                m_input->consumeContent();
                std::lock_guard<std::mutex> lock(m_mutex);
                m_requests.add(_Move(text));
                if (m_session)
                    m_session->requestControl();
                m_ready.notify_one();
            }
            else if (!m_input->isActive())
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_closed = true;
                if (m_session)
                    m_session->requestControl();
                m_ready.notify_one();
                break;
            }
        }
    }

    bool receive(String& text, bool wait)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (wait)
            m_ready.wait(lock, [&] { return m_closed || m_requests.getCount(); });
        if (!m_requests.getCount())
        {
            if (m_closed)
                m_disconnected = true;
            return false;
        }
        text = _Move(m_requests[0]);
        m_requests.removeAt(0);
        return true;
    }

    void send(DAPMessage& message)
    {
        message.number(message.root, "seq", m_sequence++);
        String text = message.serialize(message.root);
        if (SLANG_FAILED(m_output->write(text.getBuffer(), text.getLength())))
            m_disconnected = true;
    }

    void respond(DAPMessage& request, JSONValue body, const String& error = String())
    {
        auto response = JSONValue::makeEmptyObject();
        request.put(response, "type", "response");
        request.number(response, "request_seq", request.integer(request.root, "seq"));
        request.put(response, "command", request.string(request.root, "command"));
        request.flag(response, "success", !error.getLength());
        if (error.getLength())
            request.put(response, "message", error);
        else if (body.isValid())
            request.put(response, "body", body);
        request.root = response;
        send(request);
    }

    void event(const char* name)
    {
        DAPMessage message;
        message.put(message.root, "type", "event");
        message.put(message.root, "event", name);
        send(message);
    }

    void output(const String& text, const char* category)
    {
        DAPMessage message;
        message.put(message.root, "type", "event");
        message.put(message.root, "event", "output");
        auto body = JSONValue::makeEmptyObject();
        message.put(body, "category", category);
        message.put(body, "output", text);
        message.put(message.root, "body", body);
        send(message);
    }

    void invalidateReferences()
    {
        m_frames.clear();
        m_frameIds.clear();
        m_variables.clear();
    }

    JSONValue source(DAPMessage& message, const String& path)
    {
        auto result = JSONValue::makeEmptyObject();
        String absolute;
        if (SLANG_FAILED(Path::getCanonical(path, absolute)))
            absolute = path;
        message.put(result, "path", absolute);
        message.put(result, "name", Path::getFileName(path));
        return result;
    }

    JSONValue variable(DAPMessage& message, const DebugValueInfo& value, bool evaluation)
    {
        auto result = JSONValue::makeEmptyObject();
        if (!evaluation)
            message.put(result, "name", value.name);
        message.put(result, evaluation ? "result" : "value", value.value);
        message.put(result, "type", value.typeName);
        int32_t handle = 0;
        if (value.reference)
        {
            handle = m_nextHandle++;
            m_variables.add(
                handle,
                {-1, false, value.reference, value.namedChildren, value.indexedChildren});
        }
        if (value.isTextureMip)
            message.put(result, "__vscodeVariableMenuContext", "hgvmTextureMip");
        message.number(result, "variablesReference", handle);
        message.number(result, "namedVariables", value.namedChildren);
        message.number(result, "indexedVariables", value.indexedChildren);
        return result;
    }

    /// Creates an export only for an existing stop-scoped reference, and removes partial files.
    void exportTextureMip(DAPMessage& message, JSONValue args, String& error)
    {
        VariableReference reference;
        int64_t handle = message.integer(args, "variablesReference");
        if (handle <= 0 || handle > INT32_MAX ||
            !m_variables.tryGetValue(int32_t(handle), reference) || !reference.value)
        {
            error = "Invalid or expired variable reference.";
            return;
        }
        String path = message.string(args, "destinationPath");
        if (!path.getLength() || path.indexOf('\0') >= 0 || !Path::isAbsolute(path))
        {
            error = "Texture export requires an absolute destinationPath.";
            return;
        }
        struct FileWriter : SequentialByteWriter
        {
            FileStream stream;
            SlangResult write(std::span<const uint8_t> bytes) override
            {
                return stream.write(bytes.data(), bytes.size());
            }
        } writer;
        if (SLANG_FAILED(
                writer.stream.init(path, FileMode::CreateNew, FileAccess::Write, FileShare::None)))
        {
            error = "Cannot create texture export (destination must not already exist).";
            return;
        }
        SlangResult result = m_session->exportTextureMip(reference.value, writer, error);
        if (SLANG_SUCCEEDED(result) && SLANG_FAILED(writer.stream.flush()))
            error = "Failed to flush the texture export.";
        writer.stream.close();
        if (SLANG_FAILED(result) || error.getLength())
            File::remove(path);
    }

    bool launch(DAPMessage& message, JSONValue args, String& error)
    {
        if (m_launched)
        {
            error = "This adapter already has a program. Start a new debug session to run again.";
            return false;
        }
        String program = message.string(args, "program");
        String cwd = message.string(args, "cwd");
        if (!program.getLength())
        {
            error = "launch requires a program path.";
            return false;
        }
        if (message.boolean(args, "noDebug"))
        {
            error = "Use slangi directly to run without debugging.";
            return false;
        }
        if (cwd.getLength())
        {
            std::error_code result;
            std::filesystem::current_path(std::filesystem::u8path(cwd.getBuffer()), result);
            if (result)
            {
                error = "Cannot open the launch working directory.";
                return false;
            }
        }
        String absolute;
        if (SLANG_FAILED(Path::getCanonical(program, absolute)) || !File::exists(absolute))
        {
            error = "Cannot open program: " + program;
            return false;
        }
        program = absolute;
        String apiName = message.string(args, "api", "default");
        RenderApiType api = apiName == "default"
                                ? RenderApiType::Unknown
                                : RenderApiUtil::findApiTypeByName(apiName.getUnownedSlice());
        if (api == RenderApiType::Unknown && apiName != "default")
        {
            error = "Unknown RHI API: " + apiName;
            return false;
        }
        m_entry = message.string(args, "entryPoint", "main");
        m_stopOnEntry = message.boolean(args, "stopOnEntry");
        GPUExecutionMode gpuExecution = GPUExecutionMode::Default;
        auto executionArgument = message.get(args, "gpuExecution");
        if (executionArgument.isValid() &&
            (executionArgument.getKind() != JSONValue::Kind::String ||
             !parseGPUExecutionMode(message.json.getString(executionArgument), gpuExecution)))
        {
            error = "gpuExecution must be default, buffered, or synchronous.";
            return false;
        }
        ComPtr<slang::IBlob> code;
        if (Path::getPathExt(program.getUnownedSlice()) == toSlice("hgbc"))
        {
            List<uint8_t> bytes;
            if (SLANG_FAILED(File::readAllBytes(program, bytes)))
            {
                error = "Cannot read HGBC program.";
                return false;
            }
            code = ListBlob::moveCreate(bytes);
        }
        else
        {
            List<String> paths;
            List<const char*> pointers;
            auto includePaths = message.get(args, "includePaths");
            if (includePaths.getKind() == JSONValue::Kind::Array)
                for (auto path : message.json.getArray(includePaths))
                    if (path.getKind() == JSONValue::Kind::String)
                        paths.add(message.json.getString(path));
            for (const auto& path : paths)
                pointers.add(path.getBuffer());
            String diagnostics;
            auto result = compileHGBCProgram(
                program.getBuffer(),
                m_entry.getBuffer(),
                pointers.getBuffer(),
                pointers.getCount(),
                SLANG_DEBUG_INFO_LEVEL_STANDARD,
                SLANG_DIAGNOSTIC_COLOR_NEVER,
                code,
                diagnostics);
            if (diagnostics.getLength())
                output(diagnostics, "stderr");
            if (SLANG_FAILED(result))
            {
                error = "Failed to compile the HGVM program.";
                return false;
            }
        }
        RefPtr<OutputSink> sink = new OutputSink(this);
        // The runtime owns the output sink and the interpreter throughout the DAP session.
        if (SLANG_FAILED(NativeHGVMRuntime::create(
                code,
                program.getBuffer(),
                cwd.getLength() ? cwd.getBuffer() : Path::getParentDirectory(program).getBuffer(),
                gpuExecution,
                sink,
                api,
                SLANG_DIAGNOSTIC_COLOR_NEVER,
                SLANG_OPTIMIZATION_LEVEL_DEFAULT,
                m_runtime,
                error,
                nullptr,
                true)))
            return false;
        auto session = m_runtime->getDebugSession();
        if (SLANG_FAILED(session->prepare(m_runtime->getProgram(), error)))
            return false;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_session = session;
            if (m_requests.getCount() || m_closed)
                m_session->requestControl();
        }
        m_launched = true;
        return true;
    }

    void runProgram()
    {
        m_running = true;
        String error;
        auto result = m_session->run(m_entry, m_stopOnEntry, this, error);
        m_running = false;
        m_finished = true;
        invalidateReferences();
        if (SLANG_FAILED(result))
        {
            m_runtime->prependDebugMessages(error);
            output(error + "\n", "stderr");
        }
        DAPMessage exited;
        exited.put(exited.root, "type", "event");
        exited.put(exited.root, "event", "exited");
        auto body = JSONValue::makeEmptyObject();
        exited.number(body, "exitCode", SLANG_FAILED(result) ? 1 : 0);
        exited.put(exited.root, "body", body);
        send(exited);
        event("terminated");
    }

    /// Handles commands only on the execution thread. A resume result ends the stopped request
    /// loop; servicing a control request while running leaves its temporary breakpoints intact.
    bool dispatch(
        const String& text,
        bool stopped,
        DebugCommand& command,
        DebugControlAction& control)
    {
        DAPMessage message;
        if (!message.parse(text) || message.string(message.root, "type") != "request")
        {
            output("Invalid DAP request.\n", "stderr");
            return false;
        }
        String name = message.string(message.root, "command");
        auto args = message.get(message.root, "arguments");
        auto body = JSONValue::makeEmptyObject();
        String error;
        bool resume = false;
        bool run = false;
        bool initializedEvent = false;
        if (name == "initialize")
        {
            if (m_initialized)
                error = "Already initialized.";
            else if (message.string(args, "pathFormat", "path") != "path")
                error = "This adapter requires native source paths.";
            else
            {
                m_initialized = true;
                m_linesStartAtOne = message.boolean(args, "linesStartAt1", true);
                m_columnsStartAtOne = message.boolean(args, "columnsStartAt1", true);
                message.flag(body, "supportsConfigurationDoneRequest", true);
                message.flag(body, "supportsFunctionBreakpoints", true);
                message.flag(body, "supportsEvaluateForHovers", true);
                message.flag(body, "supportsTerminateRequest", true);
                message.put(body, "exceptionBreakpointFilters", JSONValue::makeEmptyArray());
            }
        }
        else if (name == "disconnect" || name == "terminate")
        {
            control = DebugControlAction::Stop;
            command = DebugCommand::Stop;
            resume = true;
            m_disconnected = name == "disconnect";
            if (!m_running && !m_finished)
            {
                m_finished = true;
                event("terminated");
            }
        }
        else if (!m_initialized)
            error = "Initialize the adapter first.";
        else if (name == "launch")
            initializedEvent = launch(message, args, error);
        else if (name == "threads")
        {
            auto threads = JSONValue::makeEmptyArray();
            if (m_launched && !m_finished)
            {
                auto thread = JSONValue::makeEmptyObject();
                message.number(thread, "id", 1);
                message.put(thread, "name", "HGVM");
                message.json.addToArray(threads, thread);
            }
            message.put(body, "threads", threads);
        }
        else if (!m_launched || m_finished)
            error = "No active program.";
        else if (name == "configurationDone")
        {
            if (m_configured)
                error = "Configuration is already complete.";
            else
                run = m_configured = true;
        }
        else if (name == "setBreakpoints" || name == "setFunctionBreakpoints")
        {
            auto requested = message.get(args, "breakpoints");
            List<uint32_t> lines;
            List<String> names;
            String path = message.string(message.get(args, "source"), "path");
            if (name == "setBreakpoints" && !path.getLength())
                error = "A source path is required.";
            if (requested.getKind() == JSONValue::Kind::Array)
                for (auto item : message.json.getArray(requested))
                {
                    if (message.get(item, "condition").isValid() ||
                        message.get(item, "hitCondition").isValid() ||
                        message.get(item, "logMessage").isValid())
                        error = "Conditional breakpoints and logpoints are not supported.";
                    int64_t line = message.integer(item, "line", -1) + (m_linesStartAtOne ? 0 : 1);
                    if (name == "setBreakpoints")
                    {
                        if (line <= 0 || line > UINT32_MAX)
                            error = "Invalid breakpoint line.";
                        lines.add(uint32_t(line));
                    }
                    else
                    {
                        String function = message.string(item, "name");
                        if (!function.getLength())
                            error = "A function name is required.";
                        names.add(function);
                    }
                }
            if (!error.getLength())
            {
                auto resolved = name == "setBreakpoints"
                                    ? m_session->replaceSourceBreakpoints(path, lines)
                                    : m_session->replaceFunctionBreakpoints(names);
                auto breakpoints = JSONValue::makeEmptyArray();
                for (const auto& breakpoint : resolved)
                {
                    auto value = JSONValue::makeEmptyObject();
                    message.number(value, "id", breakpoint.id);
                    message.flag(value, "verified", breakpoint.addresses.getCount() != 0);
                    if (breakpoint.file.getLength())
                    {
                        message.put(value, "source", source(message, breakpoint.file));
                        message.number(
                            value,
                            "line",
                            breakpoint.line - (m_linesStartAtOne ? 0 : 1));
                    }
                    if (!breakpoint.addresses.getCount())
                        message.put(
                            value,
                            "message",
                            "No executable location matches this breakpoint.");
                    message.json.addToArray(breakpoints, value);
                }
                message.put(body, "breakpoints", breakpoints);
            }
        }
        else if (name == "setExceptionBreakpoints")
        {
            auto filters = message.get(args, "filters");
            if (filters.getKind() == JSONValue::Kind::Array &&
                message.json.getArray(filters).getCount())
                error = "Exception breakpoints are not supported.";
        }
        else if (name == "pause")
        {
            if (message.integer(args, "threadId", 1) != 1)
                error = "Unknown thread.";
            else if (!stopped && m_running)
                control = DebugControlAction::Pause;
        }
        else if (!stopped)
            error = "notStopped";
        else if (name == "continue" || name == "next" || name == "stepIn" || name == "stepOut")
        {
            if (message.integer(args, "threadId", 1) != 1)
                error = "Unknown thread.";
            else
            {
                bool instruction = message.string(args, "granularity") == "instruction";
                command = name == "continue"  ? DebugCommand::Continue
                          : name == "stepOut" ? DebugCommand::Finish
                          : name == "next"
                              ? (instruction ? DebugCommand::NextInstruction : DebugCommand::Next)
                              : (instruction ? DebugCommand::StepInstruction : DebugCommand::Step);
                message.flag(body, "allThreadsContinued", true);
                resume = true;
            }
        }
        else if (name == "stackTrace")
        {
            if (message.integer(args, "threadId", 1) != 1)
                error = "Unknown thread.";
            auto stack = m_session->getStackTrace();
            auto frames = JSONValue::makeEmptyArray();
            int64_t start = message.integer(args, "startFrame");
            int64_t levels = message.integer(args, "levels");
            if (start < 0 || levels < 0)
                error = "Invalid stack range.";
            else
                for (Index i = Index(start);
                     i < stack.getCount() && (!levels || i - start < levels);
                     ++i)
                {
                    const auto& frame = stack[i];
                    auto value = JSONValue::makeEmptyObject();
                    // Repeated or overlapping stack requests refer to the same logical frames
                    // throughout this stop, including when VS Code loads the stack in pages.
                    while (m_frameIds.getCount() <= i)
                    {
                        int32_t id = m_nextHandle++;
                        m_frames.add(id, m_frameIds.getCount());
                        m_frameIds.add(id);
                    }
                    int32_t id = m_frameIds[i];
                    message.number(value, "id", id);
                    message.put(
                        value,
                        "name",
                        frame.functionName + (frame.isInlined ? " (inlined)" : ""));
                    if (frame.line && frame.path.getLength())
                        message.put(value, "source", source(message, frame.path));
                    message.number(
                        value,
                        "line",
                        frame.line ? frame.line - (m_linesStartAtOne ? 0 : 1) : 0);
                    message.number(
                        value,
                        "column",
                        frame.column ? frame.column - (m_columnsStartAtOne ? 0 : 1) : 0);
                    message.json.addToArray(frames, value);
                }
            message.put(body, "stackFrames", frames);
            message.number(body, "totalFrames", stack.getCount());
        }
        else if (name == "scopes" || name == "evaluate")
        {
            Index frame = 0;
            auto frameId = message.get(args, "frameId");
            if ((frameId.isValid() || name == "scopes") &&
                !m_frames.tryGetValue(int32_t(message.integer(args, "frameId")), frame))
                error = "Invalid or expired frame reference.";
            else if (name == "scopes")
            {
                auto scopes = JSONValue::makeEmptyArray();
                for (bool arguments : {true, false})
                {
                    auto scope = JSONValue::makeEmptyObject();
                    int32_t handle = m_nextHandle++;
                    m_variables.add(handle, {frame, arguments});
                    message.put(scope, "name", arguments ? "Arguments" : "Locals");
                    message.put(scope, "presentationHint", arguments ? "arguments" : "locals");
                    message.number(scope, "variablesReference", handle);
                    message.flag(scope, "expensive", false);
                    message.json.addToArray(scopes, scope);
                }
                message.put(body, "scopes", scopes);
            }
            else
            {
                DebugValueInfo value;
                if (SLANG_SUCCEEDED(m_session->evaluateExpression(
                        frame,
                        message.string(args, "expression"),
                        value,
                        error,
                        false,
                        false)))
                    body = variable(message, value, true);
            }
        }
        else if (name == "hgvm/exportTextureMip")
            exportTextureMip(message, args, error);
        else if (name == "variables")
        {
            VariableReference reference;
            int64_t start = message.integer(args, "start"), count = message.integer(args, "count");
            if (!m_variables.tryGetValue(
                    int32_t(message.integer(args, "variablesReference")),
                    reference))
                error = "Invalid or expired variable reference.";
            else if (start < 0 || count < 0 || start > UINT32_MAX || count > UINT32_MAX)
                error = "Invalid variable range.";
            else
            {
                List<DebugValueInfo> values;
                String filter = message.string(args, "filter");
                if (reference.frame >= 0)
                {
                    if (filter != "indexed")
                        m_session->getVariables(
                            reference.frame,
                            reference.arguments,
                            values,
                            error,
                            false);
                }
                else if (
                    (filter != "indexed" || reference.indexed) &&
                    (filter != "named" || reference.named))
                    m_session->getValueChildren(
                        reference.value,
                        uint32_t(start),
                        uint32_t(count),
                        values,
                        error);
                auto variables = JSONValue::makeEmptyArray();
                for (Index i = 0; i < values.getCount(); ++i)
                    if (reference.frame < 0 || (i >= start && (!count || i - start < count)))
                        message.json.addToArray(variables, variable(message, values[i], false));
                message.put(body, "variables", variables);
            }
        }
        else
            error = "Unsupported request: " + name;
        respond(message, body, error);
        if (initializedEvent)
            event("initialized");
        if (run)
            runProgram();
        return resume && !error.getLength();
    }

    DebugCommand onStopped(DebugSession& session) override
    {
        invalidateReferences();
        DAPMessage message;
        message.put(message.root, "type", "event");
        message.put(message.root, "event", "stopped");
        auto body = JSONValue::makeEmptyObject();
        const char* reason = session.getStopReason() == DebugStopReason::Entry        ? "entry"
                             : session.getStopReason() == DebugStopReason::Breakpoint ? "breakpoint"
                             : session.getStopReason() == DebugStopReason::Pause      ? "pause"
                                                                                      : "step";
        message.put(body, "reason", reason);
        message.number(body, "threadId", 1);
        message.flag(body, "allThreadsStopped", true);
        if (session.getHitBreakpoint())
        {
            auto ids = JSONValue::makeEmptyArray();
            message.json.addToArray(ids, JSONValue::makeInt(session.getHitBreakpoint()));
            message.put(body, "hitBreakpointIds", ids);
        }
        message.put(message.root, "body", body);
        send(message);
        String text;
        DebugCommand command = DebugCommand::Stop;
        DebugControlAction control = DebugControlAction::Resume;
        while (!m_disconnected && receive(text, true))
            if (dispatch(text, true, command, control))
                break;
        invalidateReferences();
        return command;
    }

    DebugControlAction onControl(DebugSession&) override
    {
        String text;
        DebugCommand command = DebugCommand::Continue;
        DebugControlAction control = DebugControlAction::Resume;
        while (!m_disconnected && receive(text, false))
        {
            dispatch(text, false, command, control);
            if (control != DebugControlAction::Resume)
                break;
        }
        return m_disconnected ? DebugControlAction::Stop : control;
    }

public:
    ~DAPAdapter()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_shutdown = true;
            m_session = nullptr;
        }
        if (m_reader.joinable())
            m_reader.join();
        m_runtime.setNull();
    }

    SlangResult run()
    {
        RefPtr<Stream> input, output;
        SLANG_RETURN_ON_FAIL(Process::getStdStream(StdStreamType::In, input));
        SLANG_RETURN_ON_FAIL(Process::getStdStream(StdStreamType::Out, output));
        RefPtr<BufferedReadStream> buffered = new BufferedReadStream(input);
        m_input = new HTTPPacketConnection(buffered, output);
        m_output = new HTTPPacketConnection(buffered, output);
        m_reader = std::thread([this] { readRequests(); });
        String text;
        DebugCommand command = DebugCommand::Continue;
        DebugControlAction control = DebugControlAction::Resume;
        while (!m_disconnected && receive(text, true))
            dispatch(text, false, command, control);
        return SLANG_OK;
    }
};
} // namespace

SlangResult runHGVMDebugDAP()
{
    DAPAdapter adapter;
    return adapter.run();
}
} // namespace SlangITool
