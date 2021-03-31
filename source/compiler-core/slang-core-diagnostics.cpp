// slang-core-diagnostics.cpp
#include "slang-core-diagnostics.h"

namespace Slang {

namespace CoreDiagnostics
{
#define DIAGNOSTIC(id, severity, name, messageFormat) const DiagnosticInfo name = { id, Severity::severity, #name, messageFormat };
#include "slang-core-diagnostic-defs.h"
#undef DIAGNOSTIC
}

static const DiagnosticInfo* const kAllDiagnostics[] =
{
#define DIAGNOSTIC(id, severity, name, messageFormat) &CoreDiagnostics::name, 
#include "slang-core-diagnostic-defs.h"
#undef DIAGNOSTIC
};

DiagnosticsLookup* getCoreDiagnosticsLookup()
{
    static RefPtr<DiagnosticsLookup> s_lookup = new DiagnosticsLookup(kAllDiagnostics, SLANG_COUNT_OF(kAllDiagnostics));
    return s_lookup;
}

} // namespace Slang
