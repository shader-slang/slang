#pragma once

#include "slang-perfect-hash.h"
#include "slang-diagnostic-sink.h"

namespace Slang
{
    SlangResult writePerfectHashLookupCppFile(String fileName, List<String> opnames, String enumName, String enumPrefix, String enumHeaderFile, DiagnosticSink* sink);
}
