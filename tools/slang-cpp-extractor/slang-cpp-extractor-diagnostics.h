#ifndef SLANG_CPP_EXTRACTOR_DIAGNOSTICS_H
#define SLANG_CPP_EXTRACTOR_DIAGNOSTICS_H

#include "../../source/slang/slang-diagnostics.h"

namespace SlangExperimental {
using namespace Slang;

namespace CPPDiagnostics {

#define DIAGNOSTIC(id, severity, name, messageFormat) extern const DiagnosticInfo name;
#include "slang-cpp-extractor-diagnostic-defs.h"

} // CPPDiagnostics
} // SlangExperimental

#endif
