#include "core/slang-platform.h"
#include "slang-com-ptr.h"
#include "slang/slang-compiler-api.h"
#include "unit-test/slang-unit-test.h"

#include <atomic>
#include <thread>

using namespace Slang;

namespace
{

/// True unless `SLANG_ONDEMAND_IR` is explicitly "0". Mirrors `isOnDemandIRLoadEnabled()`
/// in slang-serialize-ir.cpp, which is file-local there.
bool _onDemandLoadingExpected()
{
    StringBuilder value;
    if (SLANG_FAILED(
            PlatformUtil::getEnvironmentVariable(UnownedStringSlice("SLANG_ONDEMAND_IR"), value)))
    {
        return true;
    }
    const String text = value.produceString();
    return text.getLength() == 0 || text[0] != '0';
}

} // namespace

// Checks that a decoration's own children survive on-demand loading.
//
// This guards the `inDecorationSubtree` rule in the load-time scan. Decorations are kept
// eager because the symbol index reads them without materializing anything, and a
// decoration that is itself a parent means keeping the decoration is not enough: its
// children are reachable only through it, so nothing on that path would ever trigger the
// materialization that would supply them. Keeping only the decoration inst gives back a
// decoration that silently has no children.
//
// The module under test is built directly rather than compiled from source, and the
// building happens inside slang because that is where the IR builders live. The shape does
// not occur in any module the compiler produces -- a scan over every serialized decoration
// in the builtin modules finds zero with children, and a precompiled module built from
// autodiff source has none either -- so a test driven by a shader would pass whether this
// rule were implemented or not. The IR permits the shape (decorations can be declared
// `parent = true`), which is what makes the rule worth having and worth testing, and
// building the module directly is the only way to test it that cannot go vacuous.
SLANG_UNIT_TEST(irDeferredBodyKeepsDecorationChildren)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    Index expectedChildren = 0;
    Index actualChildren = 0;
    bool bodyWasDeferred = false;
    _testRoundTripDecorationWithChildren(
        globalSession,
        expectedChildren,
        actualChildren,
        bodyWasDeferred);

    // Guards the premise: if the decoration ever stops being built with children, the
    // comparison below would hold trivially and this test would check nothing.
    SLANG_CHECK_ABORT(expectedChildren == 2);

    // Likewise: an eager load keeps everything, so it says nothing about the rule.
    if (_onDemandLoadingExpected())
        SLANG_CHECK(bodyWasDeferred);

    // The assertion the rule is about. Under the bug this replaces, the decoration comes
    // back with no children at all: they sit at the same depth as body instructions and
    // were skipped along with them, and materializing the body later does not supply them,
    // because the body's encoding starts after the decorations.
    SLANG_CHECK(actualChildren == expectedChildren);
}

// Checks that concurrent first-touch materialization of a deferred body is safe.
//
// A global session is shared across threads and holds the modules whose bodies are
// deferred, so two compiles can reach the same body at once. That is what the loader's
// mutex and the acquire/release publication of a body exist for: a body is built as a
// detached chain and attached with a single release store, and every list traversal loads
// those links with acquire, so a walker sees either no body or a complete one.
//
// The other tests here are single-threaded, which leaves that protocol unexercised.
//
// Deliberately scoped to materialization rather than to whole compiles. Compiling
// concurrently against one shared global session is documented as unsupported --
// `include/slang.h` states a global session is not thread-safe and that front-end work
// requires external synchronization -- and measurably crashes, with on-demand loading
// either on or off. A test shaped that way would be exercising unsupported usage rather
// than this mechanism, and would fail no matter what this PR did.
//
// The concurrency Slang does support is the serial-frontend/parallel-backend workflow in
// docs/user-guide/08-compiling.md, and that is clean here at 16 threads in both modes.
SLANG_UNIT_TEST(irDeferredBodyConcurrentMaterialization)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    Index deferredCount = 0;
    Index mismatches = 0;
    _testConcurrentBodyMaterialization(globalSession, deferredCount, mismatches);

    // An eager load races nothing, so it would make the assertion below meaningless.
    if (_onDemandLoadingExpected())
        SLANG_CHECK(deferredCount > 0);

    // Every thread must have seen a complete body every time. A body published before its
    // instructions were fully linked shows up here as a short child list.
    SLANG_CHECK(mismatches == 0);
}
