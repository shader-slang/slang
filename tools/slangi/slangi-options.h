// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include "core/slang-list.h"
#include "core/slang-render-api-util.h"
#include "core/slang-std-writers.h"
#include "core/slang-string.h"
#include "slang-hgvm/slang-hgvm-execution-mode.h"
#include "slang.h"

namespace SlangITool
{

struct EventOutputOptions
{
    enum class Color
    {
        Auto,
        Always,
        Never,
    };
    enum class Mode
    {
        None,
        Trace,
        Stats,
    };
    Mode mode = Mode::None;
    uint64_t traceEventLimit = UINT64_MAX;
    uint32_t statsTop = 5;
    Color color = Color::Auto;
    bool fullNames = false;

    bool isEnabled() const { return mode != Mode::None; }
};

struct Options
{
    Slang::String entryPointName = "main";
    Slang::String fileName;
    Slang::List<Slang::String> additionalSearchPaths;
    bool disassemble = false;
    bool debug = false;
    Slang::HGVM::GPUExecutionMode gpuExecution = Slang::HGVM::GPUExecutionMode::Default;
    EventOutputOptions events;
    bool useLegacyInterpreter = false;
    Slang::RenderApiType api = Slang::RenderApiType::Unknown;
    SlangDiagnosticColor diagnosticColor = SLANG_DIAGNOSTIC_COLOR_AUTO;
    SlangOptimizationLevel shaderOptimizationLevel = SLANG_OPTIMIZATION_LEVEL_DEFAULT;
    bool reportCheckpointIntermediates = false;
    int innerArgIndex = 0;
};

void writeUsage(ISlangWriter* writer);

/// Parses the execution mode shared by CLI options and DAP launch arguments.
bool parseGPUExecutionMode(Slang::UnownedStringSlice text, Slang::HGVM::GPUExecutionMode& mode);

/// Parses the command line shared by standalone and persistent-server slangi execution.
SlangResult parseOptions(
    Slang::StdWriters* stdWriters,
    int argc,
    const char* const* argv,
    Options& outOptions,
    bool& outShouldExit);

SlangDiagnosticColor resolveDiagnosticColor(SlangDiagnosticColor mode, ISlangWriter* errorWriter);

} // namespace SlangITool
