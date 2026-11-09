#include "debugger-test.h"

#include "compiler-core/slang-json-rpc.h"
#include "core/slang-http.h"
#include "core/slang-io.h"
#include "core/slang-string-escape-util.h"
#include "core/slang-string-util.h"

namespace Slang
{
/// Resolves a source-relative line expression before it is sent as a debugger command.
/// For example, `break '$file':[[@LINE+1]]` immediately before a statement keeps its breakpoint
/// attached to that statement when the surrounding test grows.
static SlangResult expandDebuggerLineNumbers(String& command, Index sourceLine)
{
    const auto marker = UnownedStringSlice::fromLiteral("[[@LINE");
    StringBuilder expanded;
    auto rest = command.getUnownedSlice();
    for (;;)
    {
        Index start = rest.indexOf(marker);
        if (start < 0)
            break;
        expanded << rest.head(start);
        rest = rest.tail(start + marker.getLength());
        Index end = rest.indexOf(UnownedStringSlice::fromLiteral("]]"));
        if (end < 0)
            return SLANG_FAIL;
        auto offsetText = rest.head(end);
        int64_t offset = 0;
        if (offsetText.getLength() && ((offsetText[0] != '+' && offsetText[0] != '-') ||
                                       SLANG_FAILED(StringUtil::parseInt64(offsetText, offset))))
            return SLANG_FAIL;
        if (offset <= -int64_t(sourceLine) || offset > int64_t(UINT32_MAX) - sourceLine)
            return SLANG_FAIL;
        expanded << int64_t(sourceLine) + offset;
        rest = rest.tail(end + 2);
    }
    expanded << rest;
    command = expanded.produceString();
    return SLANG_OK;
}

SlangResult collectDebuggerCommands(
    const String& source,
    const String& filePath,
    const String& prefix,
    List<String>& commands,
    String& error,
    bool json)
{
    String directive = prefix + "-SEND:";
    Index lineNumber = 0;
    for (auto line : LineParser(source.getUnownedSlice()))
    {
        ++lineNumber;
        line = line.trim();
        if (!line.startsWith("//"))
            continue;
        line = line.tail(2).trimStart();
        if (!line.startsWith(directive.getUnownedSlice()))
            continue;
        String command = line.tail(directive.getLength()).trimStart();
        if (SLANG_FAILED(expandDebuggerLineNumbers(command, lineNumber)))
        {
            StringBuilder message;
            message << filePath << ':' << lineNumber << ": invalid [[@LINE]] expression";
            error = message.produceString();
            return SLANG_FAIL;
        }
        StringBuilder escapedPath;
        if (json)
            StringEscapeUtil::getHandler(StringEscapeUtil::Style::JSON)
                ->appendEscaped(filePath.getUnownedSlice(), escapedPath);
        command = StringUtil::replaceAll(
            command.getUnownedSlice(),
            toSlice("$file"),
            json ? escapedPath.getUnownedSlice() : filePath.getUnownedSlice());
        // A debugger launch may import modules outside the test's directory. For example,
        // "$dirname/../../.." locates an external suite's root without assuming the runner's cwd.
        String directory = Path::getParentDirectory(filePath);
        escapedPath.clear();
        if (json)
            StringEscapeUtil::getHandler(StringEscapeUtil::Style::JSON)
                ->appendEscaped(directory.getUnownedSlice(), escapedPath);
        command = StringUtil::replaceAll(
            command.getUnownedSlice(),
            toSlice("$dirname"),
            json ? escapedPath.getUnownedSlice() : directory.getUnownedSlice());
        // The CLI reads a 4096-byte line, including its newline and terminator. Sending one
        // bounded command at a prompt also avoids filling stdin while the child writes output.
        if (!json && command.getLength() > 4094)
        {
            error = "Debugger command exceeds the CLI input-line limit";
            return SLANG_FAIL;
        }
        commands.add(json ? command : command + "\n");
    }
    if (!commands.getCount())
    {
        error = "No " + directive + " directives found";
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult runDebuggerCLI(
    const CommandLine& commandLine,
    const List<String>& commands,
    Int timeoutInMs,
    ExecuteResult& result,
    String& error)
{
    result.init();
    RefPtr<Process> process;
    if (SLANG_FAILED(Process::create(commandLine, 0, process)))
    {
        error = "Could not start the debugger: " + commandLine.toString();
        return SLANG_FAIL;
    }
    List<Byte> standardOutput, standardError;
    Stream* input = process->getStream(StdStreamType::In);
    Index sent = 0;
    Index lastPromptEnd = 0;
    const uint64_t start = Process::getClockTick();
    const uint64_t timeoutTicks = uint64_t(timeoutInMs) * Process::getClockFrequency() / 1000;
    while (!process->isTerminated())
    {
        Index previousSize = standardOutput.getCount() + standardError.getCount();
        if (SLANG_FAILED(
                StreamUtil::read(process->getStream(StdStreamType::Out), 0, standardOutput)) ||
            SLANG_FAILED(
                StreamUtil::read(process->getStream(StdStreamType::ErrorOut), 0, standardError)))
        {
            error = "Failed to read debugger output";
            break;
        }
        if (sent < commands.getCount() && standardError.getCount() > lastPromptEnd)
        {
            auto text = UnownedStringSlice(
                (const char*)standardError.getBuffer(),
                standardError.getCount());
            if (text.endsWith("(slangi) "))
            {
                lastPromptEnd = standardError.getCount();
                const auto& command = commands[sent];
                if (SLANG_FAILED(input->write(command.getBuffer(), command.getLength())))
                {
                    error = "Failed to send a debugger command";
                    break;
                }
                if (++sent == commands.getCount())
                    input->close();
            }
        }
        if (Process::getClockTick() - start >= timeoutTicks)
        {
            error = "Debugger test timed out";
            break;
        }
        if (previousSize == standardOutput.getCount() + standardError.getCount())
            Process::sleepCurrentThread(1);
    }
    input->close();
    if (!process->isTerminated())
    {
        process->kill(1);
        process->waitForTermination();
    }
    // The process has exited, so this only drains remaining pipe data.
    SlangResult readResult =
        ProcessUtil::readUntilTermination(process, &standardOutput, &standardError);
    result.resultCode = process->getReturnValue();
    StringBuilder outputText, errorText;
    StringUtil::appendStandardLines(
        UnownedStringSlice((const char*)standardOutput.getBuffer(), standardOutput.getCount()),
        outputText);
    StringUtil::appendStandardLines(
        UnownedStringSlice((const char*)standardError.getBuffer(), standardError.getCount()),
        errorText);
    result.standardOutput = outputText.produceString();
    result.standardError = errorText.produceString();
    if (!error.getLength() && SLANG_FAILED(readResult))
        error = "Failed to read remaining debugger output";
    if (!error.getLength() && sent != commands.getCount())
        error = "Debugger exited before all SEND directives were consumed";
    return error.getLength() ? SLANG_FAIL : SLANG_OK;
}

SlangResult runDebuggerDAP(
    const CommandLine& commandLine,
    const List<String>& commands,
    const String& programPath,
    Int timeoutInMs,
    ExecuteResult& result,
    String& error)
{
    result.init();
    RefPtr<Process> process;
    if (SLANG_FAILED(Process::create(commandLine, 0, process)))
    {
        error = "Could not start the DAP adapter.";
        return SLANG_FAIL;
    }
    RefPtr<BufferedReadStream> input =
        new BufferedReadStream(process->getStream(StdStreamType::Out));
    RefPtr<HTTPPacketConnection> connection =
        new HTTPPacketConnection(input, process->getStream(StdStreamType::In));
    SourceManager sources;
    sources.initialize(nullptr, nullptr);
    DiagnosticSink diagnostics;
    diagnostics.init(&sources, &JSONLexer::calcLexemeLocation);
    JSONContainer json(&sources);
    JSONValue response;
    List<String> events;
    List<Byte> stderrBytes;
    Dictionary<String, String> captures;
    StringBuilder program, transcript;
    StringEscapeUtil::getHandler(StringEscapeUtil::Style::JSON)
        ->appendEscaped(programPath.getUnownedSlice(), program);
    captures.add("program", program.produceString());
    int64_t sequence = 0;
    const uint64_t timeout = uint64_t(timeoutInMs) * Process::getClockFrequency() / 1000;
    auto field = [&](JSONValue object, const char* name)
    {
        return object.getKind() == JSONValue::Kind::Object
                   ? json.findObjectValue(object, json.getKey(UnownedStringSlice(name)))
                   : JSONValue();
    };
    // Keep full JSON responses in the transcript. Captures use numeric handles rather than
    // hardcoding the adapter's allocation order; waits make asynchronous stops deterministic.
    auto read = [&](JSONValue& message, uint64_t start) -> bool
    {
        for (;;)
        {
            if (SLANG_FAILED(StreamUtil::read(
                    process->getStream(StdStreamType::ErrorOut),
                    0,
                    stderrBytes)) ||
                SLANG_FAILED(connection->update()))
            {
                error = "Invalid DAP framing or failed output read.";
                return false;
            }
            if (connection->hasContent())
            {
                auto bytes = connection->getContent();
                String text(UnownedStringSlice((const char*)bytes.getBuffer(), bytes.getCount()));
                connection->consumeContent();
                transcript << text << '\n';
                if (SLANG_FAILED(JSONRPCUtil::parseJSON(
                        text.getUnownedSlice(),
                        &json,
                        &diagnostics,
                        message)))
                {
                    error = "Invalid DAP JSON.";
                    return false;
                }
                return true;
            }
            if (!connection->isActive() || Process::getClockTick() - start >= timeout)
            {
                error = "DAP adapter exited or timed out before the expected message.";
                return false;
            }
            Process::sleepCurrentThread(1);
        }
    };
    for (const auto& script : commands)
    {
        String command = script.getUnownedSlice().trim();
        if (command.startsWith("capture "))
        {
            List<UnownedStringSlice> words;
            StringUtil::splitOnWhitespace(command.getUnownedSlice(), words);
            if (words.getCount() != 3)
            {
                error = "DAP capture requires a name and a dotted response path.";
                break;
            }
            List<UnownedStringSlice> path;
            StringUtil::split(words[2], '.', path);
            JSONValue value = response;
            for (auto part : path)
            {
                if (value.getKind() == JSONValue::Kind::Object)
                    value = json.findObjectValue(value, json.getKey(part));
                else if (value.getKind() == JSONValue::Kind::Array)
                {
                    int64_t index;
                    auto array = json.getArray(value);
                    value = SLANG_SUCCEEDED(StringUtil::parseInt64(part, index)) && index >= 0 &&
                                    index < array.getCount()
                                ? array[Index(index)]
                                : JSONValue();
                }
                else
                    value = JSONValue();
            }
            if (value.getKind() != JSONValue::Kind::Integer)
            {
                error = "DAP capture did not select an integer: " + String(words[2]);
                break;
            }
            captures.set(String(words[1]), String(json.asInteger(value)));
            continue;
        }
        const uint64_t start = Process::getClockTick();
        if (command.startsWith("wait "))
        {
            String wanted = command.getUnownedSlice().tail(5).trim();
            while (!events.contains(wanted) && !error.getLength())
            {
                JSONValue message;
                if (!read(message, start))
                    break;
                auto event = field(message, "event");
                if (event.getKind() == JSONValue::Kind::String)
                    events.add(json.getString(event));
                else
                    error = "Unexpected response while waiting for a DAP event.";
            }
            if (error.getLength())
                break;
            events.removeAt(events.indexOf(wanted));
            continue;
        }
        for (const auto& capture : captures)
            command = StringUtil::replaceAll(
                command.getUnownedSlice(),
                (String("$") + capture.first).getUnownedSlice(),
                capture.second.getUnownedSlice());
        JSONValue request;
        if (SLANG_FAILED(
                JSONRPCUtil::parseJSON(command.getUnownedSlice(), &json, &diagnostics, request)) ||
            request.getKind() != JSONValue::Kind::Object)
        {
            error = "Invalid JSON in DAP-SEND.";
            break;
        }
        json.setKeyValue(request, json.getKey(toSlice("seq")), JSONValue::makeInt(++sequence));
        json.setKeyValue(
            request,
            json.getKey(toSlice("type")),
            json.createString(toSlice("request")));
        JSONWriter writer(JSONWriter::IndentationStyle::KNR);
        json.traverseRecursively(request, &writer);
        if (SLANG_FAILED(connection->write(
                writer.getBuilder().getBuffer(),
                writer.getBuilder().getLength())))
        {
            error = "Failed to send DAP request.";
            break;
        }
        for (;;)
        {
            JSONValue message;
            if (!read(message, start))
                break;
            auto event = field(message, "event");
            if (event.getKind() == JSONValue::Kind::String)
                events.add(json.getString(event));
            else
            {
                auto id = field(message, "request_seq");
                if (id.getKind() != JSONValue::Kind::Integer || json.asInteger(id) != sequence)
                    error = "Unexpected DAP response sequence.";
                response = message;
                break;
            }
        }
        if (error.getLength())
            break;
    }
    process->getStream(StdStreamType::In)->close();
    const uint64_t endStart = Process::getClockTick();
    while (!process->isTerminated() && !error.getLength())
    {
        StreamUtil::read(process->getStream(StdStreamType::ErrorOut), 0, stderrBytes);
        if (Process::getClockTick() - endStart >= timeout)
            error = "DAP adapter did not exit after stdin closed.";
        Process::sleepCurrentThread(1);
    }
    if (!process->isTerminated())
    {
        process->kill(1);
        process->waitForTermination();
    }
    List<Byte> remainder;
    ProcessUtil::readUntilTermination(process, &remainder, &stderrBytes);
    result.resultCode = process->getReturnValue();
    result.standardOutput = transcript.produceString();
    result.standardError =
        String(UnownedStringSlice((const char*)stderrBytes.getBuffer(), stderrBytes.getCount()));
    return error.getLength() ? SLANG_FAIL : SLANG_OK;
}
} // namespace Slang
