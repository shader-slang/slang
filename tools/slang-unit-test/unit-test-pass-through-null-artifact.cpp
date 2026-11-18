// unit-test-pass-through-null-artifact.cpp

#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-downstream-compiler.h"
#include "slang-compiler.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// A downstream compile that fails may leave its output artifact unset (the contract on
// IDownstreamCompiler::compile leaves that to the implementation), and callers that hit such a
// failure flow the null artifact to passthroughDownstreamDiagnostics. The function must treat a
// null artifact as "no diagnostics" rather than dereference it — this guards the shared fix on
// every platform, independent of any downstream toolkit.
SLANG_UNIT_TEST(passThroughDownstreamDiagnosticsNullArtifact)
{
    DiagnosticSink sink;
    ComPtr<IDownstreamCompiler> compiler;
    SLANG_CHECK(SLANG_SUCCEEDED(passthroughDownstreamDiagnostics(&sink, compiler, nullptr)));
    SLANG_CHECK(sink.getErrorCount() == 0);
}
