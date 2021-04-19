#ifndef CPP_EXTRACT_DIAGNOSTICS_H
#define CPP_EXTRACT_DIAGNOSTICS_H

#include "../../source/slang/slang-diagnostics.h"

namespace CppExtract {
using namespace Slang;

namespace CPPDiagnostics {

#define DIAGNOSTIC(id, severity, name, messageFormat) extern const DiagnosticInfo name;
#include "diagnostic-defs.h"

} // CPPDiagnostics
} // CppExtract

#endif
