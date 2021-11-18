#ifndef TEST_SERVER_DIAGNOSTICS_H
#define TEST_SERVER_DIAGNOSTICS_H

#include "../../source/core/slang-basic.h"
#include "../../source/core/slang-writer.h"

#include "../../source/compiler-core/slang-source-loc.h"
#include "../../source/compiler-core/slang-diagnostic-sink.h"

namespace TestServer {
using namespace Slang;

namespace ServerDiagnostics {

#define DIAGNOSTIC(id, severity, name, messageFormat) extern const DiagnosticInfo name;
#include "test-server-diagnostic-defs.h"

} // ServerDiagnostics
} // TestServer

#endif
