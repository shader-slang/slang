// unit-test-artifact-diagnostics.cpp

#include "compiler-core/slang-artifact-associated-impl.h"
#include "compiler-core/slang-artifact.h"
#include "slang-com-ptr.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// getRaw() on an ArtifactDiagnostics whose raw text was never set must return a valid,
// NUL-terminated empty slice rather than dereference a null buffer. An empty StringBuilder yields a
// null-begin UnownedStringSlice, so SliceUtil::asTerminatedCharSlice has to special-case zero
// length; otherwise TerminatedCharSlice(nullptr, 0) runs SLANG_ASSERT(in[0] == 0) on a null
// pointer. Every downstream compiler's diagnostics flow through getRaw(), so this guards the shared
// compiler-core fix on every platform, independent of any downstream toolkit.
SLANG_UNIT_TEST(artifactDiagnosticsGetRawEmpty)
{
    ComPtr<IArtifactDiagnostics> diagnostics = ArtifactDiagnostics::create();
    TerminatedCharSlice raw = diagnostics->getRaw();
    SLANG_CHECK_ABORT(raw.begin() != nullptr);
    SLANG_CHECK(raw.begin()[0] == 0);
}
