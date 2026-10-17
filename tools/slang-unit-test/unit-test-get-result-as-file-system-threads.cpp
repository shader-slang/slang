// unit-test-get-result-as-file-system-threads.cpp

#include "core/slang-io.h"
#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <atomic>
#include <thread>
#include <vector>

using namespace Slang;

// `getResultAsFileSystem` adds associations to the entry point's cached result artifact if they are
// missing: here an obfuscated source map (the session enables obfuscation, so the module has one).
// Calling it from several threads on the same linked program, so that they share one artifact, used
// to race on the artifact's association list (a check-then-add with no lock), which could add an
// association twice or corrupt the list. `getEntryPointMetadata` reads the same list, so some
// threads call it at the same time.

// Count the files (and the source-map files, `*.map`) in `directory` of `fileSystem`, recursively.
static void _countFiles(
    ISlangMutableFileSystem* fileSystem,
    const String& directory,
    int& outFileCount,
    int& outSourceMapCount)
{
    struct Contents
    {
        String directory;
        List<String> files;
        List<String> directories;
    };
    Contents contents;
    contents.directory = directory;
    fileSystem->enumeratePathContents(
        directory.getBuffer(),
        [](SlangPathType pathType, const char* name, void* userData)
        {
            auto contents = static_cast<Contents*>(userData);
            String path = Path::combine(contents->directory, name);
            if (pathType == SLANG_PATH_TYPE_DIRECTORY)
                contents->directories.add(path);
            else
                contents->files.add(path);
        },
        &contents);
    for (const auto& file : contents.files)
    {
        ++outFileCount;
        if (file.endsWith(".map"))
            ++outSourceMapCount;
    }
    for (const auto& subDirectory : contents.directories)
        _countFiles(fileSystem, subDirectory, outFileCount, outSourceMapCount);
}

// Returns true if `fileSystem` contains files and exactly one source map.
static bool _hasOneSourceMap(ISlangMutableFileSystem* fileSystem)
{
    int fileCount = 0;
    int sourceMapCount = 0;
    _countFiles(fileSystem, ".", fileCount, sourceMapCount);
    return fileCount > 0 && sourceMapCount == 1;
}

SLANG_UNIT_TEST(getResultAsFileSystemParallel)
{
    const char* userSourceBody = R"(
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID, uniform RWStructuredBuffer<int> o)
        {
            o[0] = 1;
        }
        )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    constexpr int kRoundCount = 20;
    constexpr int kThreadCount = 8;
    constexpr int kMetadataReadCount = 20;

    int failureCount = 0;
    for (int round = 0; round < kRoundCount; ++round)
    {
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_HLSL;
        targetDesc.profile = globalSession->findProfile("sm_6_0");
        slang::CompilerOptionEntry obfuscateOption = {};
        obfuscateOption.name = slang::CompilerOptionName::Obfuscate;
        obfuscateOption.value.kind = slang::CompilerOptionValueKind::Int;
        obfuscateOption.value.intValue0 = 1;
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        sessionDesc.compilerOptionEntries = &obfuscateOption;
        sessionDesc.compilerOptionEntryCount = 1;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnostics;
        auto module = session->loadModuleFromSourceString(
            "m",
            "m.slang",
            userSourceBody,
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);

        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_CHECK_ABORT(
            module->findEntryPointByName("computeMain", entryPoint.writeRef()) == SLANG_OK);

        slang::IComponentType* components[] = {module, entryPoint.get()};
        ComPtr<slang::IComponentType> composite;
        SLANG_CHECK_ABORT(
            session->createCompositeComponentType(
                components,
                2,
                composite.writeRef(),
                diagnostics.writeRef()) == SLANG_OK);
        ComPtr<slang::IComponentType> linked;
        SLANG_CHECK_ABORT(composite->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

        // Compile once up front so that all threads work on the same cached artifact.
        ComPtr<slang::IBlob> code;
        SLANG_CHECK_ABORT(
            linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()) == SLANG_OK);

        std::atomic<bool> go{false};
        std::atomic<int> failures{0};
        std::vector<std::thread> threads;
        for (int i = 0; i < kThreadCount; ++i)
        {
            const bool readMetadata = (i % 2) == 1;
            threads.emplace_back(
                [&, readMetadata]()
                {
                    while (!go.load())
                        std::this_thread::yield();
                    if (readMetadata)
                    {
                        for (int read = 0; read < kMetadataReadCount; ++read)
                        {
                            ComPtr<slang::IMetadata> metadata;
                            if (SLANG_FAILED(
                                    linked->getEntryPointMetadata(0, 0, metadata.writeRef())) ||
                                !metadata)
                                failures++;
                        }
                        return;
                    }
                    ComPtr<ISlangMutableFileSystem> fileSystem;
                    if (SLANG_FAILED(linked->getResultAsFileSystem(0, 0, fileSystem.writeRef())) ||
                        !fileSystem || !_hasOneSourceMap(fileSystem))
                        failures++;
                });
        }
        go = true;
        for (auto& thread : threads)
            thread.join();
        failureCount += failures.load();

        // After the parallel calls the artifact must still hold exactly one source map.
        ComPtr<ISlangMutableFileSystem> fileSystem;
        SLANG_CHECK(linked->getResultAsFileSystem(0, 0, fileSystem.writeRef()) == SLANG_OK);
        SLANG_CHECK(fileSystem && _hasOneSourceMap(fileSystem));
    }
    SLANG_CHECK(failureCount == 0);
}
