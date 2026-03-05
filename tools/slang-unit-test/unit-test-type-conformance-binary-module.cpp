// unit-test-type-conformance-binary-module.cpp

// Test that type conformances work when modules are loaded from precompiled binary blobs.
// This reproduces: https://github.com/shader-slang/slang/issues/10371

#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Slang;

SLANG_UNIT_TEST(typeConformanceBinaryModule)
{
    const char* fooSource = R"(
        module foo;
        public interface IFoo {
            uint get();
        }
        public struct Foo1 : IFoo {
            uint dummy;
            uint get() { return 1; }
        }
        public struct Foo2 : IFoo {
            uint dummy;
            uint get() { return 2; }
        }
    )";

    const char* testSource = R"(
        module test;
        import foo;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid: SV_DispatchThreadID, RWStructuredBuffer<uint> result)
        {
            uint i = tid.x;
            uint type_id = tid.x;
            uint dummy = 0;
            IFoo f = createDynamicObject<IFoo>(type_id, dummy);
            result[i] = f.get();
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::CompilerOptionEntry optEntry;
    optEntry.name = slang::CompilerOptionName::Optimization;
    optEntry.value.kind = slang::CompilerOptionValueKind::Int;
    optEntry.value.intValue0 = 0;
    targetDesc.compilerOptionEntryCount = 1;
    targetDesc.compilerOptionEntries = &optEntry;

    // Phase 1: compile from source, serialize both modules to binary blobs.
    ComPtr<ISlangBlob> fooBlob;
    ComPtr<ISlangBlob> testBlob;
    {
        ComPtr<ISlangFileSystemExt> fs = ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
        auto& memFS = *static_cast<MemoryFileSystem*>(fs.get());
        memFS.saveFile("foo.slang", fooSource, strlen(fooSource));
        memFS.saveFile("test.slang", testSource, strlen(testSource));

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        sessionDesc.fileSystem = fs;
        const char* searchPaths[] = {"."};
        sessionDesc.searchPathCount = 1;
        sessionDesc.searchPaths = searchPaths;

        ComPtr<slang::ISession> session;
        SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<ISlangBlob> diagnostics;
        auto testModule = session->loadModule("test", diagnostics.writeRef());
        SLANG_CHECK(testModule != nullptr);

        for (int i = 0; i < session->getLoadedModuleCount(); i++)
        {
            auto mod = session->getLoadedModule(i);
            ComPtr<ISlangBlob> serialized;
            SLANG_CHECK(mod->serialize(serialized.writeRef()) == SLANG_OK);
            String name = mod->getName();
            if (name == "foo")
                fooBlob = serialized;
            else if (name == "test")
                testBlob = serialized;
        }
        SLANG_CHECK(fooBlob != nullptr);
        SLANG_CHECK(testBlob != nullptr);
    }

    // Phase 2: load both modules from binary blobs in a new session.
    // Then look up types from the imported module and create type conformances.
    {
        ComPtr<ISlangFileSystemExt> fs = ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
        auto& memFS = *static_cast<MemoryFileSystem*>(fs.get());
        memFS.saveFileBlob("foo.slang-module", fooBlob);
        memFS.saveFileBlob("test.slang-module", testBlob);

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        sessionDesc.fileSystem = fs;
        const char* searchPaths[] = {"."};
        sessionDesc.searchPathCount = 1;
        sessionDesc.searchPaths = searchPaths;

        ComPtr<slang::ISession> session;
        SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<ISlangBlob> diagnostics;
        auto testModule = session->loadModule("test", diagnostics.writeRef());
        SLANG_CHECK(testModule != nullptr);

        ComPtr<slang::IEntryPoint> entryPoint;
        testModule->findAndCheckEntryPoint(
            "computeMain",
            SLANG_STAGE_COMPUTE,
            entryPoint.writeRef(),
            diagnostics.writeRef());
        SLANG_CHECK(entryPoint != nullptr);

        // Look up types defined in the imported module via reflection.
        auto layout = testModule->getLayout();
        SLANG_CHECK(layout != nullptr);

        auto ifoo = layout->findTypeByName("IFoo");
        auto foo1 = layout->findTypeByName("Foo1");
        auto foo2 = layout->findTypeByName("Foo2");
        SLANG_CHECK(ifoo != nullptr);
        SLANG_CHECK(foo1 != nullptr);
        SLANG_CHECK(foo2 != nullptr);

        // Create type conformances for dynamic dispatch.
        ComPtr<slang::ITypeConformance> foo1IFoo;
        ComPtr<slang::ITypeConformance> foo2IFoo;
        SLANG_CHECK(
            session->createTypeConformanceComponentType(
                foo1,
                ifoo,
                foo1IFoo.writeRef(),
                0,
                diagnostics.writeRef()) == SLANG_OK);
        SLANG_CHECK(
            session->createTypeConformanceComponentType(
                foo2,
                ifoo,
                foo2IFoo.writeRef(),
                1,
                diagnostics.writeRef()) == SLANG_OK);

        // Compose all component types and link.
        slang::IComponentType* componentTypes[] =
            {testModule, entryPoint.get(), foo1IFoo, foo2IFoo};
        ComPtr<slang::IComponentType> composedProgram;
        SLANG_CHECK(
            session->createCompositeComponentType(
                componentTypes,
                SLANG_COUNT_OF(componentTypes),
                composedProgram.writeRef(),
                diagnostics.writeRef()) == SLANG_OK);

        ComPtr<slang::IComponentType> linkedProgram;
        SLANG_CHECK(
            composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> code;
        linkedProgram->getTargetCode(0, code.writeRef(), diagnostics.writeRef());
        SLANG_CHECK(code != nullptr);
    }
}
