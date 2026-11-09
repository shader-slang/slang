#pragma once

#include "core/slang-process-util.h"

namespace Slang
{
/// Extracts PREFIX-SEND comments, expanding $file, $dirname, and [[@LINE+N]] locations.
SlangResult collectDebuggerCommands(
    const String& source,
    const String& filePath,
    const String& prefix,
    List<String>& commands,
    String& error,
    bool json = false);

/// Drives the CLI through its prompts, drains both output streams, and closes stdin after the
/// final command. A timeout or early exit leaves the partial output available for diagnostics.
SlangResult runDebuggerCLI(
    const CommandLine& commandLine,
    const List<String>& commands,
    Int timeoutInMs,
    ExecuteResult& result,
    String& error);

/// Sends JSON requests, waits for named events, and captures integer response fields for later
/// requests. The same FileCheck transcript is used for source and precompiled programs.
SlangResult runDebuggerDAP(
    const CommandLine& commandLine,
    const List<String>& commands,
    const String& programPath,
    Int timeoutInMs,
    ExecuteResult& result,
    String& error);
} // namespace Slang
