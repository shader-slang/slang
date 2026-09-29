// unit-test-diagnostic-callback.cpp
//
// Tests for ISession::setDiagnosticCallback and the SlangStructuredDiagnosticCallback /
// SlangStructuredDiagnostic API.

#include "core/slang-basic.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Helper: create a plain session with a single SPIRV-ASM target (no GPU needed).
static ComPtr<slang::ISession> makeSession(slang::IGlobalSession* gs)
{
    slang::TargetDesc target = {};
    target.format = SLANG_SPIRV_ASM;
    target.profile = gs->findProfile("sm_5_0");

    slang::SessionDesc desc = {};
    desc.targetCount = 1;
    desc.targets = &target;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(gs->createSession(desc, session.writeRef()) == SLANG_OK);
    return session;
}

struct CapturedDiagnostic
{
    SlangSeverity severity;
    int64_t code;
    String message;
    String filename;
    uint32_t startLine;
    uint32_t startCol;
    uint32_t endLine;
    uint32_t endCol;
    uint32_t secondarySpanCount;
    // Only the first secondary span is captured; that is enough to exercise and assert on
    // the pointer-lifetime handling in Linkage::structuredDiagnosticThunk without needing to
    // copy the whole array.
    String secondaryFilename;
    String secondaryMessage;
    uint32_t secondaryStartLine;
};

// Callback that appends each diagnostic to a caller-supplied list.
static bool collectCallback(const SlangStructuredDiagnostic* d, void* userData)
{
    auto* list = static_cast<List<CapturedDiagnostic>*>(userData);
    CapturedDiagnostic c;
    c.severity = d->severity;
    c.code = d->code;
    c.message = d->message ? d->message : "";
    c.filename = d->primarySpan.filename ? d->primarySpan.filename : "";
    c.startLine = d->primarySpan.startLine;
    c.startCol = d->primarySpan.startCol;
    c.endLine = d->primarySpan.endLine;
    c.endCol = d->primarySpan.endCol;
    c.secondarySpanCount = d->secondarySpanCount;
    if (d->secondarySpanCount > 0)
    {
        const SlangDiagnosticSpan& sec = d->secondarySpans[0];
        c.secondaryFilename = sec.filename ? sec.filename : "";
        c.secondaryMessage = sec.message ? sec.message : "";
        c.secondaryStartLine = sec.startLine;
    }
    list->add(c);
    return true;
}

// Callback that counts invocations.
static bool countCallback(const SlangStructuredDiagnostic*, void* userData)
{
    (*static_cast<int*>(userData))++;
    return true;
}

// 1. Callback fires and reports the correct severity for a sema error.
SLANG_UNIT_TEST(diagnosticCallbackErrorSeverity)
{
    ComPtr<slang::IGlobalSession> gs;
    SLANG_CHECK_ABORT(slang_createGlobalSession(SLANG_API_VERSION, gs.writeRef()) == SLANG_OK);
    auto session = makeSession(gs);

    List<CapturedDiagnostic> captured;
    session->setDiagnosticCallback(collectCallback, &captured);

    // 'undefined' is an undeclared identifier — guaranteed to produce an error.
    const char* src = "int x = undefined;";
    ComPtr<slang::IBlob> diag;
    auto* mod = session->loadModuleFromSourceString("m", "m.slang", src, diag.writeRef());
    // Module may be null (error) — that's fine, we only care about callback invocations.
    SLANG_UNUSED(mod);

    SLANG_CHECK(captured.getCount() > 0);
    // 30015 is E30015 ("undefined identifier"), the specific diagnostic 'undefined' is expected
    // to provoke. This may cascade into further errors (e.g. a placeholder error for the
    // resulting ErrorType), so look for E30015 specifically rather than asserting every
    // error-severity diagnostic carries this code.
    bool sawExpectedError = false;
    for (auto& c : captured)
        if (c.severity == SLANG_SEVERITY_ERROR && c.code == 30015)
            sawExpectedError = true;
    SLANG_CHECK(sawExpectedError);
}

// 2. Callback fires with SLANG_SEVERITY_WARNING for a warning (not an error).
SLANG_UNIT_TEST(diagnosticCallbackWarningSeverity)
{
    ComPtr<slang::IGlobalSession> gs;
    SLANG_CHECK_ABORT(slang_createGlobalSession(SLANG_API_VERSION, gs.writeRef()) == SLANG_OK);
    auto session = makeSession(gs);

    List<CapturedDiagnostic> captured;
    session->setDiagnosticCallback(collectCallback, &captured);

    // A non-static global variable at module scope ('globalUnused' below) is an implicit
    // uniform, which triggers warning E39019 ("global-uniform-not-expected") once the module
    // is compiled to a target — not at loadModule() time, which only parses and type-checks.
    const char* src = R"(
        int globalUnused;
        [shader("compute")]
        [numthreads(1,1,1)]
        void main() {}
    )";
    ComPtr<slang::IBlob> modDiag;
    auto* mod = session->loadModuleFromSourceString("m", "m.slang", src, modDiag.writeRef());
    SLANG_CHECK_ABORT(mod != nullptr);

    ComPtr<slang::IComponentType> linked;
    ComPtr<slang::IBlob> linkDiag;
    mod->link(linked.writeRef(), linkDiag.writeRef());
    SLANG_CHECK_ABORT(linked != nullptr);

    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> targetDiag;
    linked->getTargetCode(0, code.writeRef(), targetDiag.writeRef());

    // 39019 is E39019 ("global-uniform-not-expected"), the specific diagnostic 'globalUnused'
    // is expected to provoke; look for it specifically rather than asserting every
    // warning-severity diagnostic carries this code.
    bool sawExpectedWarning = false;
    for (auto& c : captured)
        if (c.severity == SLANG_SEVERITY_WARNING && c.code == 39019)
            sawExpectedWarning = true;
    SLANG_CHECK(sawExpectedWarning);
}

// 3. Callback and blob agree: every diagnostic the callback sees also appears in the blob.
SLANG_UNIT_TEST(diagnosticCallbackAgreesWithBlob)
{
    ComPtr<slang::IGlobalSession> gs;
    SLANG_CHECK_ABORT(slang_createGlobalSession(SLANG_API_VERSION, gs.writeRef()) == SLANG_OK);
    auto session = makeSession(gs);

    List<CapturedDiagnostic> captured;
    session->setDiagnosticCallback(collectCallback, &captured);

    const char* src = "int x = undefined;";
    ComPtr<slang::IBlob> blob;
    session->loadModuleFromSourceString("m", "m.slang", src, blob.writeRef());

    // There must be at least one captured diagnostic.
    SLANG_CHECK(captured.getCount() > 0);
    // The blob must be non-null (errors were produced).
    SLANG_CHECK(blob != nullptr);

    // Every message the callback saw must appear as a substring in the blob text.
    String blobText((const char*)blob->getBufferPointer());
    for (auto& c : captured)
    {
        if (c.message.getLength() > 0)
            SLANG_CHECK(blobText.indexOf(c.message.getUnownedSlice()) != -1);
    }
}

// 4. Clearing the callback (nullptr) stops invocations.
SLANG_UNIT_TEST(diagnosticCallbackClear)
{
    ComPtr<slang::IGlobalSession> gs;
    SLANG_CHECK_ABORT(slang_createGlobalSession(SLANG_API_VERSION, gs.writeRef()) == SLANG_OK);
    auto session = makeSession(gs);

    int count = 0;
    session->setDiagnosticCallback(countCallback, &count);
    // Verify it fires at least once with a real error.
    const char* src = "int x = undefined;";
    ComPtr<slang::IBlob> d1;
    session->loadModuleFromSourceString("m1", "m1.slang", src, d1.writeRef());
    SLANG_CHECK(count > 0);

    // Clear the callback and compile another broken module.
    session->setDiagnosticCallback(nullptr, nullptr);
    int countBefore = count;
    ComPtr<slang::IBlob> d2;
    session->loadModuleFromSourceString("m2", "m2.slang", src, d2.writeRef());
    // Count must not have increased.
    SLANG_CHECK(count == countBefore);
}

// 5. Primary span carries a valid filename and a non-zero line number.
SLANG_UNIT_TEST(diagnosticCallbackSpanLocation)
{
    ComPtr<slang::IGlobalSession> gs;
    SLANG_CHECK_ABORT(slang_createGlobalSession(SLANG_API_VERSION, gs.writeRef()) == SLANG_OK);
    auto session = makeSession(gs);

    List<CapturedDiagnostic> captured;
    session->setDiagnosticCallback(collectCallback, &captured);

    const char* src = "int x = undefined;";
    ComPtr<slang::IBlob> d;
    session->loadModuleFromSourceString("m", "my-file.slang", src, d.writeRef());

    bool sawLocatedError = false;
    for (auto& c : captured)
    {
        if (c.severity >= SLANG_SEVERITY_ERROR && c.startLine > 0)
        {
            // The filename should contain the path we passed in.
            SLANG_CHECK(c.filename.indexOf(toSlice("my-file")) != -1);
            // The end location, resolved separately from the start location in the thunk, must
            // be on the same or a later line/column — never before the start.
            SLANG_CHECK(c.endLine >= c.startLine);
            if (c.endLine == c.startLine)
                SLANG_CHECK(c.endCol >= c.startCol);
            sawLocatedError = true;
        }
    }
    SLANG_CHECK(sawLocatedError);
}

// 6. Callback fires for target-stage diagnostics (exercises slang-linkable.cpp injection sites).
SLANG_UNIT_TEST(diagnosticCallbackTargetStage)
{
    ComPtr<slang::IGlobalSession> gs;
    SLANG_CHECK_ABORT(slang_createGlobalSession(SLANG_API_VERSION, gs.writeRef()) == SLANG_OK);
    auto session = makeSession(gs);

    int count = 0;
    session->setDiagnosticCallback(countCallback, &count);

    // Global uniform produces E39019 warning during getTargetCode, not during loadModule.
    const char* src = R"(
        int globalUnused;
        [shader("compute")]
        [numthreads(1,1,1)]
        void main() {}
    )";
    ComPtr<slang::IBlob> modDiag;
    auto* mod = session->loadModuleFromSourceString("m", "m.slang", src, modDiag.writeRef());
    SLANG_CHECK_ABORT(mod != nullptr);

    ComPtr<slang::IComponentType> linked;
    mod->link(linked.writeRef(), nullptr);
    SLANG_CHECK_ABORT(linked != nullptr);

    // Snapshot the count after link() (not just after loadModule()) so the assertion below
    // isolates diagnostics produced specifically by getTargetCode(), rather than potentially
    // being satisfied by a diagnostic from link() instead.
    int countBeforeTarget = count;

    ComPtr<slang::IBlob> code, targetDiag;
    linked->getTargetCode(0, code.writeRef(), targetDiag.writeRef());

    // The target stage must have added at least one more callback invocation.
    SLANG_CHECK(count > countBeforeTarget);
}

// 7. Registering a new callback replaces the previous one: only the newest callback fires,
// with its own userData.
SLANG_UNIT_TEST(diagnosticCallbackReplacement)
{
    ComPtr<slang::IGlobalSession> gs;
    SLANG_CHECK_ABORT(slang_createGlobalSession(SLANG_API_VERSION, gs.writeRef()) == SLANG_OK);
    auto session = makeSession(gs);

    int countA = 0;
    int countB = 0;
    session->setDiagnosticCallback(countCallback, &countA);
    session->setDiagnosticCallback(countCallback, &countB);

    const char* src = "int x = undefined;";
    ComPtr<slang::IBlob> d;
    session->loadModuleFromSourceString("m", "m.slang", src, d.writeRef());

    // Only the most recently registered callback (with userData == &countB) fired.
    SLANG_CHECK(countA == 0);
    SLANG_CHECK(countB > 0);
}

// 8. A diagnostic disabled via CompilerOptionName::DisableWarning does not reach the callback,
// confirming the callback reports severity "after overrides are applied" as documented.
SLANG_UNIT_TEST(diagnosticCallbackSeverityOverride)
{
    ComPtr<slang::IGlobalSession> gs;
    SLANG_CHECK_ABORT(slang_createGlobalSession(SLANG_API_VERSION, gs.writeRef()) == SLANG_OK);

    slang::TargetDesc target = {};
    target.format = SLANG_SPIRV_ASM;
    target.profile = gs->findProfile("sm_5_0");

    // Disable E39019 ("global-uniform-not-expected") for this session, the same diagnostic
    // exercised by diagnosticCallbackWarningSeverity above.
    slang::CompilerOptionEntry disableWarning = {};
    disableWarning.name = slang::CompilerOptionName::DisableWarning;
    disableWarning.value.kind = slang::CompilerOptionValueKind::String;
    disableWarning.value.stringValue0 = "39019";

    slang::SessionDesc desc = {};
    desc.targetCount = 1;
    desc.targets = &target;
    desc.compilerOptionEntries = &disableWarning;
    desc.compilerOptionEntryCount = 1;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(gs->createSession(desc, session.writeRef()) == SLANG_OK);

    List<CapturedDiagnostic> captured;
    session->setDiagnosticCallback(collectCallback, &captured);

    const char* src = R"(
        int globalUnused;
        [shader("compute")]
        [numthreads(1,1,1)]
        void main() {}
    )";
    ComPtr<slang::IBlob> modDiag;
    auto* mod = session->loadModuleFromSourceString("m", "m.slang", src, modDiag.writeRef());
    SLANG_CHECK_ABORT(mod != nullptr);

    ComPtr<slang::IComponentType> linked;
    mod->link(linked.writeRef(), nullptr);
    SLANG_CHECK_ABORT(linked != nullptr);

    ComPtr<slang::IBlob> code, targetDiag;
    linked->getTargetCode(0, code.writeRef(), targetDiag.writeRef());

    // The disabled diagnostic must never reach the callback, at any severity.
    for (auto& c : captured)
        SLANG_CHECK(c.code != 39019);
}

// 9. A diagnostic with a secondary span (E30515, "generic-param-shadows-outer-generic") reaches
// the callback with a populated secondarySpans array. This exercises the pointer-lifetime
// handling in Linkage::structuredDiagnosticThunk (the secBeginLocs/secSpans reserve()+add()
// loop that keeps each span's filename buffer valid) — every other test here uses a
// single-span diagnostic, so this loop otherwise never runs.
SLANG_UNIT_TEST(diagnosticCallbackSecondarySpans)
{
    ComPtr<slang::IGlobalSession> gs;
    SLANG_CHECK_ABORT(slang_createGlobalSession(SLANG_API_VERSION, gs.writeRef()) == SLANG_OK);
    auto session = makeSession(gs);

    List<CapturedDiagnostic> captured;
    session->setDiagnosticCallback(collectCallback, &captured);

    // The inner generic parameter 'T' on bar() shadows the outer generic parameter 'T' on Foo,
    // producing E30515 with a primary span at the inner declaration and a secondary span at the
    // outer declaration ("outer generic parameter 'T' declared here").
    const char* src = R"(
        struct Foo<T>
        {
            T bar<T>(T x) { return x; }
        }
    )";
    ComPtr<slang::IBlob> diag;
    auto* mod = session->loadModuleFromSourceString("m", "m.slang", src, diag.writeRef());
    SLANG_UNUSED(mod);

    bool sawSecondarySpan = false;
    for (auto& c : captured)
    {
        if (c.code == 30515 && c.secondarySpanCount > 0)
        {
            SLANG_CHECK(c.secondaryFilename.getLength() > 0);
            SLANG_CHECK(c.secondaryStartLine > 0);
            SLANG_CHECK(c.secondaryMessage.getLength() > 0);
            sawSecondarySpan = true;
        }
    }
    SLANG_CHECK(sawSecondarySpan);
}
