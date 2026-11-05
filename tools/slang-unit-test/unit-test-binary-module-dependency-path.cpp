// unit-test-binary-module-dependency-path.cpp
//
// Tests for how the file dependencies recorded in a `.slang-module` are found again when the
// module is loaded from that binary instead of from source.

#include "core/slang-io.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

namespace
{

struct TempDir
{
    String path;

    ~TempDir()
    {
        if (path.getLength())
            Path::removeNonEmpty(path);
    }
};

/// Creates a fresh temporary directory and stores its canonical path in `out`. The path is
/// canonical so that it compares equal to the canonical dependency paths the test checks for.
SlangResult makeTempDir(const char* prefix, TempDir& out)
{
    String base;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice(prefix), base));
    // `generateTemporary` creates a placeholder file; we replace it with a directory of the same
    // unique name.
    SLANG_RETURN_ON_FAIL(File::remove(base));
    if (!Path::createDirectoryRecursive(base))
        return SLANG_FAIL;
    out.path = base;
    return Path::getCanonical(base, out.path);
}

/// Creates a fresh directory, named like a temporary file, directly under the working directory and
/// stores its canonical path in `out`. A module source inside it is recorded relative to the
/// working directory without any `..`, so the recorded path names no file relative to any other
/// directory the test uses.
SlangResult makeWorkingDirectoryTempDir(const char* prefix, TempDir& out)
{
    String temporary;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice(prefix), temporary));
    SLANG_RETURN_ON_FAIL(File::remove(temporary));
    const String path = Path::combine(Path::getCurrentPath(), Path::getFileName(temporary));
    if (!Path::createDirectoryRecursive(path))
        return SLANG_FAIL;
    out.path = path;
    return Path::getCanonical(path, out.path);
}

SlangResult writeFile(const String& path, const char* text)
{
    if (!Path::createDirectoryRecursive(Path::getParentDirectory(path)))
        return SLANG_FAIL;
    return File::writeAllText(path, text);
}

SlangResult createSession(
    slang::IGlobalSession* globalSession,
    const String& root,
    ComPtr<slang::ISession>& outSession)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::CompilerOptionEntry useUpToDateBinaryModule = {};
    useUpToDateBinaryModule.name = slang::CompilerOptionName::UseUpToDateBinaryModule;
    useUpToDateBinaryModule.value.kind = slang::CompilerOptionValueKind::Int;
    useUpToDateBinaryModule.value.intValue0 = 1;

    const String searchPaths[] = {
        Path::combine(root, "cache/session"),
        Path::combine(root, "shadow/pkg/shaders"),
        Path::combine(root, "prod/shaders"),
    };
    const char* searchPathPtrs[] = {
        searchPaths[0].getBuffer(),
        searchPaths[1].getBuffer(),
        searchPaths[2].getBuffer(),
    };

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.searchPaths = searchPathPtrs;
    sessionDesc.searchPathCount = SLANG_COUNT_OF(searchPathPtrs);
    sessionDesc.compilerOptionEntries = &useUpToDateBinaryModule;
    sessionDesc.compilerOptionEntryCount = 1;
    return globalSession->createSession(sessionDesc, outSession.writeRef());
}

bool dependsOnFile(slang::IModule* module, const String& canonicalPath)
{
    for (SlangInt32 i = 0; i < module->getDependencyFileCount(); i++)
    {
        const char* dependencyPath = module->getDependencyFilePath(i);
        String canonicalDependencyPath;
        if (dependencyPath &&
            SLANG_SUCCEEDED(Path::getCanonical(dependencyPath, canonicalDependencyPath)) &&
            canonicalDependencyPath == canonicalPath)
            return true;
    }
    return false;
}

SlangResult linkConsumer(
    slang::ISession* session,
    ComPtr<slang::IModule>& outConsumer,
    ComPtr<slang::IBlob>& outEntryPointHash)
{
    const char* consumerSource = R"(
        import "pkg/base";
        import "pkg/support";
        RWStructuredBuffer<int> output;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 id : SV_DispatchThreadID)
        {
            output[id.x] = baseValue() + supportValue();
        }
    )";

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> support(session->loadModule("pkg/support", diagnostics.writeRef()));
    if (!support)
        return SLANG_FAIL;
    ComPtr<slang::IModule> consumer(session->loadModuleFromSourceString(
        "consumer",
        "consumer.slang",
        consumerSource,
        diagnostics.writeRef()));
    if (!consumer)
        return SLANG_FAIL;
    outConsumer = consumer;

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_RETURN_ON_FAIL(consumer->findEntryPointByName("computeMain", entryPoint.writeRef()));
    slang::IComponentType* components[] = {consumer, entryPoint};
    ComPtr<slang::IComponentType> composite;
    SLANG_RETURN_ON_FAIL(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        composite.writeRef()));
    ComPtr<slang::IComponentType> linked;
    SLANG_RETURN_ON_FAIL(composite->link(linked.writeRef()));
    linked->getEntryPointHash(0, 0, outEntryPointHash.writeRef());
    return outEntryPointHash ? SLANG_OK : SLANG_FAIL;
}

bool areBlobsEqual(slang::IBlob* a, slang::IBlob* b)
{
    return a->getBufferSize() == b->getBufferSize() &&
           memcmp(a->getBufferPointer(), b->getBufferPointer(), a->getBufferSize()) == 0;
}

} // namespace

// A serialized module records each file dependency after its own source file relative to the
// directory of that source file, so reloading it must resolve those entries against the same
// directory and not against the include search paths.
//
// Consider this layout, with the search paths `cache/session`, `shadow/pkg/shaders` and
// `prod/shaders`:
//
//     prod/build/common.slang        static const int HelperConstant = 7;
//     prod/shaders/pkg/base.slang    #include "../../build/common.slang"
//     prod/shaders/pkg/support.slang #include "../../build/common.slang"
//     shadow/build/common.slang      a byte-identical copy of prod/build/common.slang
//     cache/session/pkg/base.slang-module
//
// `pkg/base` records `../../build/common.slang` for its include. Relative to the search path
// `shadow/pkg/shaders` that spelling names `shadow/build/common.slang`, which is a different file
// from the one `pkg/base` was compiled against. When the binary module is reloaded, the include
// must still be `prod/build/common.slang`: otherwise a consumer that also imports `pkg/support`
// sees the helper under two identities and its entry-point hash changes, and an edit to the real
// helper goes unnoticed by the up-to-date check because the copy still matches the recorded
// digest.
SLANG_UNIT_TEST(binaryModuleDependencyResolvesRelativeToModuleSource)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    TempDir root;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(makeTempDir("slang-binary-module-dep", root)));

    const char* helperSource = "static const int HelperConstant = 7;\n";
    const String prodHelper = Path::combine(root.path, "prod/build/common.slang");
    const String shadowHelper = Path::combine(root.path, "shadow/build/common.slang");
    const String cachedBaseModule = Path::combine(root.path, "cache/session/pkg/base.slang-module");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeFile(prodHelper, helperSource)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeFile(shadowHelper, helperSource)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeFile(
        Path::combine(root.path, "prod/shaders/pkg/base.slang"),
        "#include \"../../build/common.slang\"\n"
        "public int baseValue() { return HelperConstant; }\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeFile(
        Path::combine(root.path, "prod/shaders/pkg/support.slang"),
        "#include \"../../build/common.slang\"\n"
        "public int supportValue() { return HelperConstant; }\n")));
    SLANG_CHECK_ABORT(
        Path::createDirectoryRecursive(Path::combine(root.path, "shadow/pkg/shaders")));
    String canonicalProdHelper, canonicalShadowHelper;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(Path::getCanonical(prodHelper, canonicalProdHelper)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(Path::getCanonical(shadowHelper, canonicalShadowHelper)));

    ComPtr<slang::IBlob> baseModuleBlob;
    SlangInt32 sourceDependencyCount = 0;
    ComPtr<slang::IBlob> sourceEntryPointHash;
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(createSession(globalSession, root.path, session)));
        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> base(session->loadModule("pkg/base", diagnostics.writeRef()));
        SLANG_CHECK_ABORT(base != nullptr);
        SLANG_CHECK_ABORT(UnownedStringSlice(base->getFilePath()).endsWith(".slang"));
        SLANG_CHECK(dependsOnFile(base, canonicalProdHelper));

        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(base->serialize(baseModuleBlob.writeRef())));
        SLANG_CHECK_ABORT(
            Path::createDirectoryRecursive(Path::getParentDirectory(cachedBaseModule)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllBytes(
            cachedBaseModule,
            baseModuleBlob->getBufferPointer(),
            baseModuleBlob->getBufferSize())));

        ComPtr<slang::IModule> consumer;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(linkConsumer(session, consumer, sourceEntryPointHash)));
        SLANG_CHECK(dependsOnFile(consumer, canonicalProdHelper));
        SLANG_CHECK(!dependsOnFile(consumer, canonicalShadowHelper));
        sourceDependencyCount = consumer->getDependencyFileCount();
        // The consumer, `pkg/base`, `pkg/support` and the shared helper.
        SLANG_CHECK(sourceDependencyCount == 4);
    }

    // The second session reuses the cached binary for `pkg/base`. Its include resolves to the
    // file it was compiled against, so the consumer sees the same dependencies and hash.
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(createSession(globalSession, root.path, session)));
        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> base(session->loadModule("pkg/base", diagnostics.writeRef()));
        SLANG_CHECK_ABORT(base != nullptr);
        SLANG_CHECK_ABORT(UnownedStringSlice(base->getFilePath()).endsWith(".slang-module"));
        SLANG_CHECK(dependsOnFile(base, canonicalProdHelper));
        SLANG_CHECK(!dependsOnFile(base, canonicalShadowHelper));

        ComPtr<slang::IModule> consumer;
        ComPtr<slang::IBlob> binaryEntryPointHash;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(linkConsumer(session, consumer, binaryEntryPointHash)));
        SLANG_CHECK(dependsOnFile(consumer, canonicalProdHelper));
        SLANG_CHECK(!dependsOnFile(consumer, canonicalShadowHelper));
        SLANG_CHECK(consumer->getDependencyFileCount() == sourceDependencyCount);
        SLANG_CHECK(areBlobsEqual(binaryEntryPointHash, sourceEntryPointHash));
    }

    // After the real helper changes, the cached binary is stale even though the copy under the
    // shadow root still matches the digest recorded for it, so `pkg/base` loads from source.
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(writeFile(prodHelper, "static const int HelperConstant = 8;\n")));
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(createSession(globalSession, root.path, session)));
        SLANG_CHECK(!session->isBinaryModuleUpToDate(cachedBaseModule.getBuffer(), baseModuleBlob));

        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> base(session->loadModule("pkg/base", diagnostics.writeRef()));
        SLANG_CHECK_ABORT(base != nullptr);
        SLANG_CHECK(UnownedStringSlice(base->getFilePath()).endsWith(".slang"));
    }
}

// A module that is not under any search directory records its own source file relative to the
// working directory, and the up-to-date check finds it there, so an edit to one of its includes
// makes the cached binary stale rather than letting it pass as a standalone module. Loading the
// module again by the same path finds that source file the same way.
SLANG_UNIT_TEST(binaryModuleSourceOutsideSearchPathsIsChecked)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    TempDir root;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(makeWorkingDirectoryTempDir("slang-binary-module-cwd", root)));

    const String moduleSource = Path::combine(root.path, "src/foo.slang");
    const String include = Path::combine(root.path, "src/inc.slang");
    const String cachedModule = Path::combine(root.path, "out/foo.slang-module");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeFile(include, "static const int K = 7;\n")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeFile(
        moduleSource,
        "module foo;\n#include \"inc.slang\"\npublic int f() { return K; }\n")));
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(Path::getParentDirectory(cachedModule)));
    for (const char* searchPath : {"cache/session", "shadow/pkg/shaders", "prod/shaders"})
        SLANG_CHECK_ABORT(Path::createDirectoryRecursive(Path::combine(root.path, searchPath)));

    ComPtr<slang::IBlob> moduleBlob;
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(createSession(globalSession, root.path, session)));
        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> module(
            session->loadModule(moduleSource.getBuffer(), diagnostics.writeRef()));
        SLANG_CHECK_ABORT(module != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->serialize(moduleBlob.writeRef())));
    }

    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(createSession(globalSession, root.path, session)));
        SLANG_CHECK(session->isBinaryModuleUpToDate(cachedModule.getBuffer(), moduleBlob));
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(writeFile(include, "static const int K = 8;\n")));
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(createSession(globalSession, root.path, session)));
        SLANG_CHECK(!session->isBinaryModuleUpToDate(cachedModule.getBuffer(), moduleBlob));
    }
}
