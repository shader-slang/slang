#include "slang-cpp-extractor-diagnostics.h"

namespace SlangExperimental {

namespace CPPDiagnostics
{
using namespace Slang;

#define DIAGNOSTIC(id, severity, name, messageFormat) const DiagnosticInfo name = { id, Severity::severity, #name, messageFormat };
#include "slang-cpp-extractor-diagnostic-defs.h"
}

} // namespace SlangExperimental
