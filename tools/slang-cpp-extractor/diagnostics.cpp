#include "diagnostics.h"

namespace CppExtract {

namespace CPPDiagnostics
{
using namespace Slang;

#define DIAGNOSTIC(id, severity, name, messageFormat) const DiagnosticInfo name = { id, Severity::severity, #name, messageFormat };
#include "diagnostic-defs.h"
}

} // namespace CppExtract
