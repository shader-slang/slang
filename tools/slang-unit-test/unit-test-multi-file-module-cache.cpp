// unit-test-multi-file-module-cache.cpp

#include "core/slang-io.h"
#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test loading a precompiled module that imports other modules.
SLANG_UNIT_TEST(multiFileModuleCache)
{
    // Prepare the virtual file system with the module files.
    const char* commonAttrSource = R"(
        module attrs;
        public struct MyType { int x; }
        public void doThing() {}
    )";
    const char* debugSource = R"(
        module debug;
        import common.attrs;
        struct Wrapper { MyType t; }
        [shader("compute")]
        [numthreads(1,1,1)]
        void computeMain(uint3 workGroup : SV_GroupID)
        {
            doThing();
        }
    )";
    ComPtr<ISlangFileSystemExt> fs = ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
    auto& memoryFS = *static_cast<MemoryFileSystem*>(fs.get());
    memoryFS.createDirectory("root");
    memoryFS.createDirectory("root/common");
    memoryFS.saveFile("root/debug.slang", debugSource, strlen(debugSource));
    memoryFS.saveFile("root/common/attrs.slang", commonAttrSource, strlen(commonAttrSource));

    ComPtr<ISlangFileSystemExt> precompiledFs = ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
    auto& memoryPrecompiledFS = *static_cast<MemoryFileSystem*>(precompiledFs.get());
    memoryPrecompiledFS.createDirectory("root1");
    memoryPrecompiledFS.createDirectory("root1/common");

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_6_0");
    sessionDesc.targets = &targetDesc;

    // Phase 1: load debug module and save precompiled IR modules.
    {
        ComPtr<slang::ISession> session;
        sessionDesc.fileSystem = fs;
        sessionDesc.searchPathCount = 1;
        const char* searchPaths[] = {"root"};
        sessionDesc.searchPaths = searchPaths;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangBlob> diagnostics;

        auto debug = session->loadModule("debug", diagnostics.writeRef());

        SLANG_CHECK_ABORT(debug != nullptr);
        if (diagnostics)
        {
            // If there are diagnostics, they should be warnings or errors
            SLANG_CHECK_ABORT(diagnostics->getBufferSize() > 0);
        }

        for (int i = 0; i < session->getLoadedModuleCount(); i++)
        {
            slang::IModule* mod = session->getLoadedModule(i);
            // path = root1/...
            String path = mod->getFilePath();
            path = String("root1") +
                   path.getUnownedSlice().tail(path.indexOf(Path::kOSCanonicalPathDelimiter));
            path.append("-module");

            ComPtr<slang::IBlob> serializedBlob;
            SLANG_CHECK_ABORT(mod->serialize(serializedBlob.writeRef()) == SLANG_OK);
            memoryPrecompiledFS.saveFileBlob(path.getBuffer(), serializedBlob);
        }
    }

    // Phase 2: Create a new session and load from precompiled modules.
    {
        ComPtr<slang::ISession> session;
        sessionDesc.searchPathCount = 1;
        const char* searchPaths[] = {"root1"};
        sessionDesc.searchPaths = searchPaths;
        sessionDesc.fileSystem = precompiledFs;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangBlob> diagnostics;
        auto module = session->loadModule("debug", diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);

        // Generate code.
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findEntryPointByName("computeMain", entryPoint.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);
        ComPtr<slang::IComponentType> linkedProgram;
        entryPoint->link(linkedProgram.writeRef());
        SLANG_CHECK_ABORT(linkedProgram != nullptr);
        ComPtr<ISlangBlob> code;
        linkedProgram->getEntryPointCode(0, 0, code.writeRef());
        SLANG_CHECK_ABORT(code != nullptr);
        auto codeStr =
            UnownedStringSlice((const char*)code->getBufferPointer(), code->getBufferSize());
        SLANG_CHECK(codeStr.indexOf(toSlice("computeMain")) != -1);
    }

    // Phase 3: Check that we can fail gracefully if an imported module is missing.
    {
        memoryPrecompiledFS.remove("root1/common/attrs.slang-module");

        ComPtr<slang::ISession> session;
        sessionDesc.searchPathCount = 1;
        const char* searchPaths[] = {"root1"};
        sessionDesc.searchPaths = searchPaths;
        sessionDesc.fileSystem = precompiledFs;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangBlob> diagnostics;
        auto module = session->loadModule("debug", diagnostics.writeRef());
        SLANG_CHECK_ABORT(module == nullptr);
        auto errMsg = UnownedStringSlice(
            (const char*)diagnostics->getBufferPointer(),
            diagnostics->getBufferSize());
        SLANG_CHECK(errMsg.indexOf(toSlice("error")) != -1);
    }
    // Phase 4: Check that we can fail gracefully if an imported module is out-of-date/incompatible.
    // Create an incompatible common/attrs module.
    {
        ComPtr<slang::ISession> session;
        sessionDesc.fileSystem = fs;
        sessionDesc.searchPathCount = 1;
        const char* searchPaths[] = {"root"};
        sessionDesc.searchPaths = searchPaths;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangBlob> diagnostics;
        const char* commonAttrSource1 = R"(
        module attrs;
        // public struct MyType { int x; }
        public void doThing() {}
        )";
        auto attrModule1 = session->loadModuleFromSourceString(
            "attrs",
            "common/attrs.slang",
            commonAttrSource1,
            diagnostics.writeRef());

        SLANG_CHECK_ABORT(attrModule1 != nullptr);
        if (diagnostics)
        {
            // If there are diagnostics, they should be warnings or errors
            SLANG_CHECK_ABORT(diagnostics->getBufferSize() > 0);
        }

        ComPtr<slang::IBlob> serializedBlob;
        SLANG_CHECK_ABORT(attrModule1->serialize(serializedBlob.writeRef()) == SLANG_OK);
        memoryPrecompiledFS.saveFileBlob("root1/common/attrs.slang-module", serializedBlob);
    }
    // Now try load precompiled "debug" module, and see if we can fail gracefully with an error
    // message.
    {
        ComPtr<slang::ISession> session;
        sessionDesc.searchPathCount = 1;
        const char* searchPaths[] = {"root1"};
        sessionDesc.searchPaths = searchPaths;
        sessionDesc.fileSystem = precompiledFs;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangBlob> diagnostics;
        auto module = session->loadModule("debug", diagnostics.writeRef());
        SLANG_CHECK_ABORT(module == nullptr);
        auto errMsg = UnownedStringSlice(
            (const char*)diagnostics->getBufferPointer(),
            diagnostics->getBufferSize());
        SLANG_CHECK(errMsg.indexOf(toSlice("cannot resolve imported declaration")) != -1);
    }
}
