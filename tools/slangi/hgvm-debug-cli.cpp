#include "hgvm-debug-cli.h"

#include "core/slang-io.h"
#include "core/slang-string-util.h"

#include <charconv>
#include <cstdio>
#include <cstring>

namespace SlangITool
{
using namespace Slang;
using namespace Slang::HGVM;

namespace
{
/// Converts a complete decimal command argument, rejecting trailing text and overflow.
bool parseNumber(const String& text, uint32_t& value)
{
    auto slice = text.getUnownedSlice();
    auto result = std::from_chars(slice.begin(), slice.end(), value);
    return result.ec == std::errc() && result.ptr == slice.end();
}

class DebugCLI : public IDebugSessionListener
{
public:
    DebugSession* session;
    String entry;
    bool quit = false;
    bool start = false;
    bool stopAtEntry = false;
    Index selectedFrame = 0;
    uint32_t remainingSteps = 0;
    DebugCommand repeatedCommand = DebugCommand::Continue;
    String previousCommand;
    uint32_t nextValue = 1;

    void printValue(const DebugValueInfo& value)
    {
        if (!value.children.getCount())
        {
            fprintf(stderr, "%s", value.value.getBuffer());
            return;
        }
        fprintf(stderr, "%s {", value.value.getBuffer());
        for (Index i = 0; i < value.children.getCount(); ++i)
        {
            if (i)
                fprintf(stderr, ", ");
            const auto& child = value.children[i];
            fprintf(stderr, "%s = ", child.name.getBuffer());
            printValue(child);
        }
        if (value.childrenTruncated)
            fprintf(stderr, ", ...");
        fprintf(stderr, "}");
    }

    DebugCommand onStopped(DebugSession& stoppedSession) override
    {
        if (remainingSteps && !stoppedSession.getHitBreakpoint())
        {
            --remainingSteps;
            return repeatedCommand;
        }
        remainingSteps = 0;
        selectedFrame = 0;
        if (uint32_t breakpoint = session->getHitBreakpoint())
            fprintf(stderr, "Breakpoint %u, ", breakpoint);
        printFrame(0);
        return prompt(true);
    }

    void printFrame(Index index)
    {
        auto frames = session->getStackTrace();
        if (index < 0 || index >= frames.getCount())
        {
            fprintf(stderr, "No such frame.\n");
            return;
        }
        const auto& frame = frames[index];
        fprintf(
            stderr,
            "#%u %s",
            uint32_t(index),
            frame.functionName.getLength() ? frame.functionName.getBuffer() : "<unknown>");
        if (frame.isInlined)
            fprintf(stderr, " (inlined)");
        if (frame.line)
            fprintf(stderr, " at %s:%u:%u", frame.path.getBuffer(), frame.line, frame.column);
        fprintf(stderr, "\n");
    }

    /// Prints the source location of a resolved breakpoint, retaining function names without
    /// debug lines. Source decoding happens only when the user requests the breakpoint table.
    void printBreakpointLocation(const DebugBreakpoint& breakpoint, uint32_t wordOffset)
    {
        const auto* location = session->getDebugMap()->findSourceLocation(wordOffset);
        const auto& function = location && location->functionName.getLength()
                                   ? location->functionName
                                   : breakpoint.function;
        if (function.getLength())
            fprintf(stderr, "in %s", function.getBuffer());
        if (location)
            fprintf(
                stderr,
                "%sat %s:%u",
                function.getLength() ? " " : "",
                location->path.getBuffer(),
                location->line);
        fprintf(stderr, "\n");
    }

    /// Lists logical breakpoints and their resolved locations using GDB's table layout.
    /// Addresses are staged program word offsets, not native pointers.
    void printBreakpoints()
    {
        if (!session->getBreakpoints().getCount())
        {
            fprintf(stderr, "No breakpoints or watchpoints.\n");
            return;
        }
        const char* rowFormat = "%-7s %-14s %-4s %-3s %-10s ";
        fprintf(stderr, rowFormat, "Num", "Type", "Disp", "Enb", "Address");
        fprintf(stderr, "What\n");
        for (const auto& breakpoint : session->getBreakpoints())
        {
            char number[24];
            snprintf(number, sizeof(number), "%u", breakpoint.id);
            Index count = breakpoint.addresses.getCount();
            const char* enabled = breakpoint.enabled ? "y" : "n";
            char address[11];
            if (count == 1)
                snprintf(address, sizeof(address), "0x%08x", breakpoint.addresses[0]);
            fprintf(
                stderr,
                rowFormat,
                number,
                "breakpoint",
                "keep",
                enabled,
                count == 1 ? address
                : count    ? "<MULTIPLE>"
                           : "<PENDING>");
            if (count == 1)
                printBreakpointLocation(breakpoint, breakpoint.addresses[0]);
            else
            {
                if (!count)
                {
                    fprintf(
                        stderr,
                        "%s",
                        breakpoint.function.getLength() ? breakpoint.function.getBuffer()
                                                        : breakpoint.file.getBuffer());
                    if (breakpoint.line)
                        fprintf(stderr, ":%u", breakpoint.line);
                }
                fprintf(stderr, "\n");
            }
            if (count > 1)
                for (Index i = 0; i < count; ++i)
                {
                    snprintf(number, sizeof(number), "%u.%u", breakpoint.id, uint32_t(i + 1));
                    snprintf(address, sizeof(address), "0x%08x", breakpoint.addresses[i]);
                    fprintf(stderr, rowFormat, number, "", "", enabled, address);
                    printBreakpointLocation(breakpoint, breakpoint.addresses[i]);
                }
        }
    }

    DebugCommand prompt(bool running)
    {
        for (;;)
        {
            fprintf(stderr, "(slangi) ");
            fflush(stderr);
            char buffer[4096];
            if (!fgets(buffer, sizeof(buffer), stdin))
            {
                quit = true;
                return DebugCommand::Stop;
            }
            buffer[strcspn(buffer, "\r\n")] = 0;
            String line = String(UnownedStringSlice(buffer).trim());
            if (!line.getLength())
                line = previousCommand;
            if (!line.getLength() || line.startsWith("#"))
                continue;
            previousCommand = line;
            Index space = -1;
            for (Index i = 0; i < line.getLength(); ++i)
                if (line[i] == ' ' || line[i] == '\t')
                {
                    space = i;
                    break;
                }
            String command = space < 0 ? line : String(line.getUnownedSlice().head(space));
            String argument =
                space < 0 ? String() : String(line.getUnownedSlice().tail(space + 1).trim());
            if (command == "quit" || command == "q")
            {
                quit = true;
                return DebugCommand::Stop;
            }
            if (command == "help" || command == "h")
            {
                fprintf(
                    stderr,
                    "run (r), start, starti, continue (c), step (s), next (n), finish (fin)\n"
                    "stepi (si), nexti (ni); stepping accepts a repeat count\n"
                    "break (b) FILE:LINE|LINE|FUNCTION, info breakpoints, delete (d) [ID]\n"
                    "enable [ID], disable [ID], backtrace (bt, where), frame (f) [N], up, down\n"
                    "info locals, info args, print (p) EXPR, whatis EXPR, list (l), quit (q)\n"
                    "Expressions: names, members, constant indices, and pointer dereferences.\n");
                continue;
            }
            if (command == "run" || command == "r" || command == "start" || command == "starti")
            {
                if (argument.getLength())
                {
                    fprintf(stderr, "Program arguments are not supported for HGVM entry points.\n");
                    continue;
                }
                start = true;
                stopAtEntry = command != "run" && command != "r";
                return DebugCommand::Stop;
            }
            if (command == "break" || command == "b")
            {
                if (!argument.getLength())
                {
                    fprintf(stderr, "Supply FILE:LINE, LINE, or FUNCTION.\n");
                    continue;
                }
                Index colon = argument.lastIndexOf(':');
                String file;
                uint32_t lineNumber = 0;
                uint32_t id;
                if (colon >= 0 &&
                    parseNumber(String(argument.getUnownedSlice().tail(colon + 1)), lineNumber))
                {
                    file = String(argument.getUnownedSlice().head(colon));
                    if (file.getLength() >= 2 && file[0] == '\'' &&
                        file[file.getLength() - 1] == '\'')
                        file = String(file.getUnownedSlice().subString(1, file.getLength() - 2));
                    id = session->addBreakpoint(file, lineNumber);
                }
                else if (parseNumber(argument, lineNumber))
                {
                    auto frames = session->getStackTrace();
                    if (selectedFrame >= frames.getCount() ||
                        !frames[selectedFrame].path.getLength())
                    {
                        fprintf(stderr, "No current source file; use FILE:LINE.\n");
                        continue;
                    }
                    id = session->addBreakpoint(frames[selectedFrame].path, lineNumber);
                }
                else
                    id = session->addFunctionBreakpoint(argument);
                for (const auto& breakpoint : session->getBreakpoints())
                    if (breakpoint.id == id)
                        fprintf(
                            stderr,
                            "Breakpoint %u (%u locations%s).\n",
                            id,
                            uint32_t(breakpoint.addresses.getCount()),
                            breakpoint.addresses.getCount() ? "" : ", pending");
                continue;
            }
            if (command == "info" &&
                (argument == "breakpoints" || argument == "break" || argument == "b"))
            {
                printBreakpoints();
                continue;
            }
            if (command == "delete" || command == "d" || command == "enable" ||
                command == "disable" || command == "dis")
            {
                uint32_t id = 0;
                if (argument.getLength() && (!parseNumber(argument, id) || !id))
                {
                    fprintf(stderr, "Expected a breakpoint number.\n");
                    continue;
                }
                List<uint32_t> ids;
                for (const auto& breakpoint : session->getBreakpoints())
                    if (!id || breakpoint.id == id)
                        ids.add(breakpoint.id);
                if (!ids.getCount())
                    fprintf(stderr, "No matching breakpoint.\n");
                for (uint32_t number : ids)
                    if (command == "delete" || command == "d")
                        session->removeBreakpoint(number);
                    else
                        session->enableBreakpoint(number, command == "enable");
                continue;
            }
            if (!running)
            {
                fprintf(stderr, "The program is not being run.\n");
                continue;
            }
            if (command == "backtrace" || command == "bt" || command == "where")
            {
                auto frames = session->getStackTrace();
                for (Index i = 0; i < frames.getCount(); ++i)
                    printFrame(i);
                continue;
            }
            if (command == "info" && (argument == "locals" || argument == "args"))
            {
                List<DebugValueInfo> values;
                String error;
                bool arguments = argument == "args";
                if (SLANG_FAILED(session->getVariables(selectedFrame, arguments, values, error)))
                    fprintf(stderr, "%s\n", error.getBuffer());
                else if (!values.getCount())
                    fprintf(stderr, arguments ? "No arguments.\n" : "No locals.\n");
                for (const auto& value : values)
                {
                    fprintf(stderr, "%s = ", value.name.getBuffer());
                    printValue(value);
                    fprintf(stderr, "\n");
                }
                continue;
            }
            if (command == "print" || command == "p" || command == "whatis")
            {
                DebugValueInfo value;
                String error;
                bool typeOnly = command == "whatis";
                if (SLANG_FAILED(
                        session
                            ->evaluateExpression(selectedFrame, argument, value, error, typeOnly)))
                    fprintf(stderr, "%s\n", error.getBuffer());
                else if (typeOnly)
                    fprintf(stderr, "type = %s\n", value.typeName.getBuffer());
                else
                {
                    fprintf(stderr, "$%u = ", nextValue++);
                    printValue(value);
                    fprintf(stderr, "\n");
                }
                continue;
            }
            if (command == "frame" || command == "f" || command == "up" || command == "down" ||
                command == "do")
            {
                uint32_t number =
                    command == "frame" || command == "f" ? uint32_t(selectedFrame) : 1;
                if (argument.getLength() && !parseNumber(argument, number))
                {
                    fprintf(stderr, "Expected a frame number or count.\n");
                    continue;
                }
                Index frame = command == "up"                        ? selectedFrame + number
                              : command == "down" || command == "do" ? selectedFrame - number
                                                                     : number;
                if (frame >= 0 && frame < session->getStackTrace().getCount())
                    selectedFrame = frame;
                printFrame(frame);
                continue;
            }
            if (command == "list" || command == "l")
            {
                if (argument.getLength())
                {
                    fprintf(stderr, "Explicit list locations are not supported yet.\n");
                    continue;
                }
                auto frames = session->getStackTrace();
                if (selectedFrame >= frames.getCount())
                    continue;
                const auto& frame = frames[selectedFrame];
                String source;
                if (!frame.line || SLANG_FAILED(File::readAllText(frame.path, source)))
                {
                    fprintf(stderr, "Source is unavailable.\n");
                    continue;
                }
                List<UnownedStringSlice> lines;
                StringUtil::split(source.getUnownedSlice(), '\n', lines);
                for (Index i = frame.line > 5 ? frame.line - 6 : 0;
                     i < lines.getCount() && i < frame.line + 4;
                     ++i)
                    fprintf(
                        stderr,
                        "%u\t%.*s\n",
                        uint32_t(i + 1),
                        int(lines[i].getLength()),
                        lines[i].begin());
                continue;
            }
            DebugCommand action;
            if (command == "continue" || command == "c" || command == "fg")
                action = DebugCommand::Continue;
            else if (command == "stepi" || command == "si")
                action = DebugCommand::StepInstruction;
            else if (command == "nexti" || command == "ni")
                action = DebugCommand::NextInstruction;
            else if (command == "step" || command == "s")
                action = DebugCommand::Step;
            else if (command == "next" || command == "n")
                action = DebugCommand::Next;
            else if (command == "finish" || command == "fin")
                action = DebugCommand::Finish;
            else
            {
                fprintf(stderr, "Undefined or unsupported command: %s\n", command.getBuffer());
                continue;
            }
            if (selectedFrame && action == DebugCommand::Finish)
            {
                fprintf(stderr, "Finishing an outer selected frame is not supported yet.\n");
                continue;
            }
            uint32_t count = 1;
            if (argument.getLength() &&
                (action == DebugCommand::Continue || action == DebugCommand::Finish ||
                 !parseNumber(argument, count) || !count))
            {
                fprintf(stderr, "Unsupported argument for this command.\n");
                continue;
            }
            remainingSteps = count - 1;
            repeatedCommand = action;
            return action;
        }
    }
};
} // namespace

SlangResult runHGVMDebugCLI(
    DebugSession* session,
    Program* program,
    const char* entry,
    String& error)
{
    SLANG_RETURN_ON_FAIL(session->prepare(program, error));
    DebugCLI cli;
    cli.session = session;
    cli.entry = entry;
    fprintf(stderr, "HGVM debugger. Type help for commands.\n");
    while (!cli.quit)
    {
        if (!cli.start)
            cli.prompt(false);
        if (cli.quit)
            break;
        cli.start = false;
        SlangResult result = session->run(cli.entry, cli.stopAtEntry, &cli, error);
        if (SLANG_FAILED(result))
            return result;
        if (!cli.start && !cli.quit)
            fprintf(
                stderr,
                session->wasTerminated() ? "Program terminated.\n" : "Program exited normally.\n");
    }
    return SLANG_OK;
}
} // namespace SlangITool
