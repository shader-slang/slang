// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "slangi-options.h"

#include "core/slang-io.h"
#include "core/slang-type-text-util.h"

#include <charconv>

namespace SlangITool
{
using namespace Slang;

void writeUsage(ISlangWriter* writer)
{
    WriterHelper output(writer);
    output.print("Slang Host-Graph Virtual Machine (Experimental)\n");
    output.print("Compile and execute Slang code on HGVM by default.\n");
    output.print("Usage: slangi [options] <filename>\n");
    output.print("Options:\n");
    output.print(
        "  -entry <name>   Specify the entry point function name to run. (default: main)\n");
    output.print("  -legacy         Use the legacy bytecode interpreter instead of HGVM.\n");
    output.print("  -hgvm           Explicitly select HGVM (the default).\n");
    output.print("  -debug          Open the HGVM debugger with GDB-style commands.\n");
    output.print("  -gpu-execution <mode>  default, buffered, or synchronous.\n");
    output.print(
        "                        Default: synchronous when debugging, buffered otherwise.\n");
    output.print(
        "  -dap            Serve the Debug Adapter Protocol on stdin/stdout (no filename).\n");
    output.print("  -disasm         Disassemble bytecode or HGBC before execution.\n");
    output.print(
        "  -trace-all      Write all HGVM events to stdout (requires event-tracking build).\n");
    output.print(
        "  -trace-n <num-events>  Write only the first N HGVM events to stdout (N >= 0).\n");
    output.print("                        The last trace option takes effect.\n");
    output.print("  -stats          Show live HGVM statistics on stderr, refreshed each second.\n");
    output.print("                  Redirected stderr receives only a final summary. Requires "
                 "event tracking.\n");
    output.print("                  Cannot be combined with tracing or -debug.\n");
    output.print(
        "  -stats-top <N>  Number of allocation sites and pipelines to show (default: 5).\n");
    output.print("  -color <mode>   Trace/statistics color: auto (default), always, or never.\n");
    output.print("  -full-names     Show module-qualified names in trace/statistics output.\n");
    output.print("  -I <path>       Add a module search path.\n");
    output.print("  -report-checkpoint-intermediates  Report AD checkpoint storage for HGVM "
                 "and its downstream shaders.\n");
    output.print("  -O<level>       Set downstream HGVM shader optimization: 0/none, 1/default, "
                 "2/high, or 3/maximal. (default: 1)\n");
    output.print("  -api <name>     Select the RHI API: default, d3d12, d3d11, vulkan, metal, "
                 "cpu, cuda, or webgpu.\n");
    output.print("  -diagnostic-color <mode>  Set diagnostic color: auto, always, or never.\n");
    output.print("  -help           Show this help message\n");
    output.print("A [host] entry point is compiled to HGBC and run on the HGVM using slang-rhi.\n");
}

bool parseGPUExecutionMode(UnownedStringSlice text, HGVM::GPUExecutionMode& mode)
{
    if (text == "default")
        mode = HGVM::GPUExecutionMode::Default;
    else if (text == "buffered")
        mode = HGVM::GPUExecutionMode::Buffered;
    else if (text == "synchronous")
        mode = HGVM::GPUExecutionMode::Synchronous;
    else
        return false;
    return true;
}

SlangDiagnosticColor resolveDiagnosticColor(SlangDiagnosticColor mode, ISlangWriter* errorWriter)
{
    if (mode != SLANG_DIAGNOSTIC_COLOR_AUTO)
        return mode;
    return errorWriter && errorWriter->isConsole() ? SLANG_DIAGNOSTIC_COLOR_ALWAYS
                                                   : SLANG_DIAGNOSTIC_COLOR_NEVER;
}

SlangResult parseOptions(
    StdWriters* stdWriters,
    int argc,
    const char* const* argv,
    Options& outOptions,
    bool& outShouldExit)
{
    outOptions = {};
    outShouldExit = false;
    if (!stdWriters || !argv || argc < 1)
        return SLANG_E_INVALID_ARG;

    ISlangWriter* standardOutput = stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT);
    WriterHelper errorWriter(stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_ERROR));
    if (argc < 2)
    {
        writeUsage(standardOutput);
        outShouldExit = true;
        return SLANG_OK;
    }

    bool hasStatsTop = false;
    bool requiresHGVM = false;
    for (int i = 1; i < argc; ++i)
    {
        UnownedStringSlice argument(argv[i]);
        if (argument == "-entry")
        {
            if (++i >= argc)
            {
                errorWriter.print("Missing value for -entry.\n");
                return SLANG_E_INVALID_ARG;
            }
            outOptions.entryPointName = argv[i];
        }
        else if (argument == "-help" || argument == "--help")
        {
            writeUsage(standardOutput);
            outShouldExit = true;
            return SLANG_OK;
        }
        else if (argument == "-disasm")
        {
            outOptions.disassemble = true;
        }
        else if (argument == "-hgvm")
        {
            requiresHGVM = true;
        }
        else if (argument == "-legacy")
        {
            outOptions.useLegacyInterpreter = true;
        }
        else if (argument == "-debug")
        {
            outOptions.debug = true;
            requiresHGVM = true;
        }
        else if (argument == "-gpu-execution")
        {
            if (++i >= argc)
            {
                errorWriter.print("Missing value for -gpu-execution.\n");
                return SLANG_E_INVALID_ARG;
            }
            if (!parseGPUExecutionMode(UnownedStringSlice(argv[i]), outOptions.gpuExecution))
            {
                errorWriter.print(
                    "Invalid -gpu-execution value '%s'; expected default, buffered, or "
                    "synchronous.\n",
                    argv[i]);
                return SLANG_E_INVALID_ARG;
            }
            requiresHGVM = true;
        }
        else if (argument == "-stats-top")
        {
            if (++i >= argc)
            {
                errorWriter.print("Missing value for -stats-top.\n");
                return SLANG_E_INVALID_ARG;
            }
            UnownedStringSlice value(argv[i]);
            auto parsed = std::from_chars(value.begin(), value.end(), outOptions.events.statsTop);
            if (parsed.ec != std::errc() || parsed.ptr != value.end() ||
                !outOptions.events.statsTop)
            {
                errorWriter.print(
                    "Invalid -stats-top value '%s'; expected a positive 32-bit integer.\n",
                    argv[i]);
                return SLANG_E_INVALID_ARG;
            }
            hasStatsTop = true;
        }
        else if (argument == "-trace-all" || argument == "-trace-n" || argument == "-stats")
        {
            uint64_t eventLimit = UINT64_MAX;
            if (argument == "-trace-n")
            {
                if (++i >= argc)
                {
                    errorWriter.print("Missing value for -trace-n.\n");
                    return SLANG_E_INVALID_ARG;
                }
                UnownedStringSlice value(argv[i]);
                auto parsed = std::from_chars(value.begin(), value.end(), eventLimit);
                if (parsed.ec != std::errc() || parsed.ptr != value.end())
                {
                    errorWriter.print(
                        "Invalid -trace-n value '%s'; expected a non-negative 64-bit integer.\n",
                        argv[i]);
                    return SLANG_E_INVALID_ARG;
                }
            }
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
            auto mode = argument == "-stats" ? EventOutputOptions::Mode::Stats
                                             : EventOutputOptions::Mode::Trace;
            if (outOptions.events.isEnabled() && outOptions.events.mode != mode)
            {
                errorWriter.print("-stats cannot be combined with -trace-all or -trace-n.\n");
                return SLANG_E_INVALID_ARG;
            }
            outOptions.events.mode = mode;
            outOptions.events.traceEventLimit = eventLimit;
#else
            errorWriter.print(
                "%.*s requires a build with SLANG_ENABLE_HGVM_EVENT_TRACKING=ON.\n",
                int(argument.getLength()),
                argument.begin());
            return SLANG_E_NOT_AVAILABLE;
#endif
        }
        else if (argument == "-full-names")
        {
            outOptions.events.fullNames = true;
        }
        else if (argument == "-color")
        {
            if (++i >= argc)
            {
                errorWriter.print("Missing value for -color.\n");
                return SLANG_E_INVALID_ARG;
            }
            UnownedStringSlice value(argv[i]);
            if (value == "auto")
                outOptions.events.color = EventOutputOptions::Color::Auto;
            else if (value == "always")
                outOptions.events.color = EventOutputOptions::Color::Always;
            else if (value == "never")
                outOptions.events.color = EventOutputOptions::Color::Never;
            else
            {
                errorWriter.print(
                    "Invalid -color value '%s'; expected auto, always, or never.\n",
                    argv[i]);
                return SLANG_E_INVALID_ARG;
            }
        }
        else if (argument == "-diagnostic-color")
        {
            if (++i >= argc)
            {
                errorWriter.print("Missing value for -diagnostic-color.\n");
                return SLANG_E_INVALID_ARG;
            }
            UnownedStringSlice value(argv[i]);
            if (value == "auto")
                outOptions.diagnosticColor = SLANG_DIAGNOSTIC_COLOR_AUTO;
            else if (value == "always")
                outOptions.diagnosticColor = SLANG_DIAGNOSTIC_COLOR_ALWAYS;
            else if (value == "never")
                outOptions.diagnosticColor = SLANG_DIAGNOSTIC_COLOR_NEVER;
            else
            {
                errorWriter.print(
                    "Invalid -diagnostic-color value '%s'; expected auto, always, or never.\n",
                    argv[i]);
                return SLANG_E_INVALID_ARG;
            }
        }
        else if (argument == "-I")
        {
            if (++i >= argc)
            {
                errorWriter.print("Missing value for -I.\n");
                return SLANG_E_INVALID_ARG;
            }
            outOptions.additionalSearchPaths.add(argv[i]);
        }
        else if (argument == "-api")
        {
            if (++i >= argc)
            {
                errorWriter.print("Missing value for -api.\n");
                return SLANG_E_INVALID_ARG;
            }
            UnownedStringSlice value(argv[i]);
            if (value == "default")
            {
                outOptions.api = RenderApiType::Unknown;
            }
            else
            {
                // Treat the unqualified Direct3D name as the current D3D12 backend while retaining
                // the standard render-API aliases such as vk, dx12, and mtl.
                outOptions.api =
                    value == "d3d" ? RenderApiType::D3D12 : RenderApiUtil::findApiTypeByName(value);
                if (outOptions.api == RenderApiType::Unknown)
                {
                    errorWriter.print("Unknown RHI API '%s'.\n", argv[i]);
                    return SLANG_E_INVALID_ARG;
                }
            }
        }
        else if (argument.startsWith("-I") && argument.getLength() > 2)
        {
            outOptions.additionalSearchPaths.add(argument.tail(2));
        }
        else if (argument == "-report-checkpoint-intermediates")
        {
            outOptions.reportCheckpointIntermediates = true;
            requiresHGVM = true;
        }
        else if (argument.startsWith("-O"))
        {
            UnownedStringSlice value = argument.tail(2);
            ValueInt parsedLevel = NameValueUtil::findValue(
                TypeTextUtil::getOptimizationLevelInfos(),
                value,
                ValueInt(-1));
            if (parsedLevel < SLANG_OPTIMIZATION_LEVEL_NONE ||
                parsedLevel > SLANG_OPTIMIZATION_LEVEL_MAXIMAL)
            {
                errorWriter.print(
                    "Invalid downstream shader optimization option '%s'; expected -O0/-Onone, "
                    "-O1/-Odefault, -O2/-Ohigh, or -O3/-Omaximal.\n",
                    argv[i]);
                return SLANG_E_INVALID_ARG;
            }
            outOptions.shaderOptimizationLevel = SlangOptimizationLevel(parsedLevel);
        }
        else if (argument.startsWith("-"))
        {
            errorWriter.print("Unknown option: %s\n", argv[i]);
            writeUsage(standardOutput);
            return SLANG_E_INVALID_ARG;
        }
        else
        {
            outOptions.fileName = argument;
            outOptions.innerArgIndex = i;
            break;
        }
    }

    if (outOptions.useLegacyInterpreter && (requiresHGVM || outOptions.events.isEnabled()))
    {
        errorWriter.print("-legacy cannot be combined with -hgvm, -debug, -gpu-execution, "
                          "-report-checkpoint-intermediates, or HGVM events.\n");
        return SLANG_E_INVALID_ARG;
    }
    if (outOptions.useLegacyInterpreter &&
        Path::getPathExt(outOptions.fileName.getUnownedSlice()) == toSlice("hgbc"))
    {
        errorWriter.print("-legacy cannot execute .hgbc files; omit -legacy to use HGVM.\n");
        return SLANG_E_INVALID_ARG;
    }
    if (outOptions.events.mode == EventOutputOptions::Mode::Stats && outOptions.debug)
    {
        errorWriter.print("-stats cannot be combined with -debug.\n");
        return SLANG_E_INVALID_ARG;
    }
    if (hasStatsTop && outOptions.events.mode != EventOutputOptions::Mode::Stats)
    {
        errorWriter.print("-stats-top requires -stats.\n");
        return SLANG_E_INVALID_ARG;
    }
    if (!outOptions.fileName.getLength())
    {
        writeUsage(standardOutput);
        outShouldExit = true;
        return SLANG_OK;
    }
    outOptions.diagnosticColor = resolveDiagnosticColor(
        outOptions.diagnosticColor,
        stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_ERROR));
    return SLANG_OK;
}

} // namespace SlangITool
