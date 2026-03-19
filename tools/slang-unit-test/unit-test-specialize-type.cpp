// unit-test-specialize-type.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// This test exercises the public `specializeType` API error path.
//
// The `LoopNode<P>` pattern is intentionally only ill-formed after `P` is concretely
// specialized. Using `__packBranch(P, EmptyNode, LoopNode<P>)` keeps module loading and
// reflection valid for the unspecialized type, but specializing `LoopNode<int>` forces the
// non-empty branch and produces an infinitely recursive type. The API should report the
// recursion diagnostic and return `nullptr` instead of leaking an exception.
SLANG_UNIT_TEST(specializeTypeInfiniteRecursion)
{
    const char* source = R"(
        struct EmptyNode
        {
            int marker;
        }

        struct LoopNode<each P>
        {
            __packBranch(P, EmptyNode, LoopNode<P>) next;
        }
        )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "specializeTypeInfiniteRecursion",
        "specializeTypeInfiniteRecursion.slang",
        source,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    auto layout = module->getLayout();
    SLANG_CHECK_ABORT(layout != nullptr);

    auto recursiveType = layout->findTypeByName("LoopNode<int>");
    SLANG_CHECK_ABORT(recursiveType != nullptr);

    diagnosticBlob = nullptr;
    auto specializedType =
        session->specializeType(recursiveType, nullptr, 0, diagnosticBlob.writeRef());
    SLANG_CHECK(specializedType == nullptr);
    SLANG_CHECK(diagnosticBlob != nullptr);
    auto diagnostics = UnownedStringSlice(
        (const char*)diagnosticBlob->getBufferPointer(),
        diagnosticBlob->getBufferSize());
    SLANG_CHECK(diagnostics.indexOf(toSlice("maximum type nesting level exceeded")) != -1);

    diagnosticBlob = nullptr;
    specializedType = layout->specializeType(recursiveType, 0, nullptr, diagnosticBlob.writeRef());
    SLANG_CHECK(specializedType == nullptr);
    SLANG_CHECK(diagnosticBlob != nullptr);
    diagnostics = UnownedStringSlice(
        (const char*)diagnosticBlob->getBufferPointer(),
        diagnosticBlob->getBufferSize());
    SLANG_CHECK(diagnostics.indexOf(toSlice("maximum type nesting level exceeded")) != -1);
}
