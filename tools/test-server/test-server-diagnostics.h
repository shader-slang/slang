#ifndef TEST_SERVER_DIAGNOSTICS_H
#define TEST_SERVER_DIAGNOSTICS_H

#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-source-loc.h"
#include "core/slang-basic.h"
#include "core/slang-writer.h"

namespace TestServer
{
using namespace Slang;

namespace ServerDiagnostics
{

#define DIAGNOSTIC(id, severity, name, messageFormat) extern const DiagnosticInfo name;
#include "test-server-diagnostic-defs.h"

} // namespace ServerDiagnostics
} // namespace TestServer

#endif
