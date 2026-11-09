#include "core/slang-io.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(serializedSourcePathStaysLogical)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));
    slang::SessionDesc desc = {};
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->createSession(desc, session.writeRef())));

    const char* sourcePath = "virtual/serialized-source.slang";
    ComPtr<ISlangBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "serializedSource",
        sourcePath,
        "public int getValue() { return 42; }",
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module);
    ComPtr<ISlangBlob> serialized;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->serialize(serialized.writeRef())));

    UnownedStringSlice bytes(
        static_cast<const char*>(serialized->getBufferPointer()),
        serialized->getBufferSize());
    String workingDirectory;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(Path::getCanonical(".", workingDirectory)));
    SLANG_CHECK(bytes.indexOf(UnownedStringSlice(sourcePath)) >= 0);
    SLANG_CHECK(bytes.indexOf(workingDirectory.getUnownedSlice()) == -1);

    // The module must still load in a fresh session after retaining its logical
    // source name instead of turning that name into a build-machine path.
    ComPtr<slang::ISession> restoredSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(globalSession->createSession(desc, restoredSession.writeRef())));
    auto restored = restoredSession->loadModuleFromIRBlob(
        "serializedSource",
        "serialized-source.slang-module",
        serialized,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(restored);
}
