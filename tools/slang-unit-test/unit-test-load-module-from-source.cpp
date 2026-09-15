// unit-test-load-module-from-source.cpp

#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>
#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test the loadModuleFromSource method and slang_loadModuleFromSource function
SLANG_UNIT_TEST(loadModuleFromSource)
{
    // Test source code with various content
    const char* testSource = R"(
        [shader("compute")]
        [numthreads(1,1,1)]
        void computeMain(uint3 workGroup : SV_GroupID)
        {
            // Simple compute shader
        }
    )";

    size_t sourceSize = strlen(testSource);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    // Test 1: Test the method version (existing functionality)
    {
        ComPtr<slang::IModule> module;
        ComPtr<ISlangBlob> diagnostics;

        // Use loadModuleFromSourceString which takes a string directly
        module = session->loadModuleFromSourceString(
            "testModule",
            "test.slang",
            testSource,
            diagnostics.writeRef());

        SLANG_CHECK(module != nullptr);
        if (diagnostics)
        {
            // If there are diagnostics, they should be warnings or errors
            SLANG_CHECK(diagnostics->getBufferSize() > 0);
        }
    }

    // Test 2: Test the new slang_loadModuleFromSource function
    {
        ComPtr<slang::IModule> module;
        ComPtr<ISlangBlob> diagnostics;

        module = slang_loadModuleFromSource(
            session,
            "testModule2",
            "test2.slang",
            testSource,
            sourceSize,
            diagnostics.writeRef());

        SLANG_CHECK(module != nullptr);
        if (diagnostics)
        {
            // If there are diagnostics, they should be warnings or errors
            SLANG_CHECK(diagnostics->getBufferSize() > 0);
        }
    }

    // Test 3: Test with invalid parameters
    {
        ComPtr<slang::IModule> module;
        ComPtr<ISlangBlob> diagnostics;

        // Test with null session
        module = slang_loadModuleFromSource(
            nullptr,
            "testModule",
            "test.slang",
            testSource,
            sourceSize,
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);

        // Test with null moduleName
        module = slang_loadModuleFromSource(
            session,
            nullptr,
            "test.slang",
            testSource,
            sourceSize,
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);

        // Test with null path
        module = slang_loadModuleFromSource(
            session,
            "testModule",
            nullptr,
            testSource,
            sourceSize,
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);
    }

    // Test 4: Test with null source and non-zero size (should fail)
    {
        ComPtr<slang::IModule> module;
        ComPtr<ISlangBlob> diagnostics;

        module = slang_loadModuleFromSource(
            session,
            "testModule",
            "test.slang",
            nullptr,
            10,
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);
    }

    // Test 5: Test with complex source code
    {
        const char* complexSource = R"(
            [shader("compute")]
            [numthreads(8,8,1)]
            void computeMain(uint3 workGroup : SV_GroupID, uint3 localID : SV_GroupThreadID)
            {
                uint2 pixelPos = workGroup.xy * uint2(8,8) + localID.xy;
                
                // Simple computation
                float result = sin(pixelPos.x * 0.1f) * cos(pixelPos.y * 0.1f);
                
                // Store result (in a real shader, this would go to a buffer)
                // outputBuffer[pixelPos] = result;
            }
        )";

        size_t complexSourceSize = strlen(complexSource);

        ComPtr<slang::IModule> module;
        ComPtr<ISlangBlob> diagnostics;

        module = slang_loadModuleFromSource(
            session,
            "complexModule",
            "complex.slang",
            complexSource,
            complexSourceSize,
            diagnostics.writeRef());

        SLANG_CHECK(module != nullptr);
    }

    // Test 6: Test IR blob functions with invalid parameters
    {
        ComPtr<slang::IModule> module;
        ComPtr<ISlangBlob> diagnostics;
        const char* testData = "test data";
        size_t testDataSize = strlen(testData);

        // Test with null session
        module = slang_loadModuleFromIRBlob(
            nullptr,
            "testModule",
            "test.slang",
            testData,
            testDataSize,
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);

        // Test with null moduleName
        module = slang_loadModuleFromIRBlob(
            session,
            nullptr,
            "test.slang",
            testData,
            testDataSize,
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);

        // Test with null path
        module = slang_loadModuleFromIRBlob(
            session,
            "testModule",
            nullptr,
            testData,
            testDataSize,
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);

        // Test with null source
        module = slang_loadModuleFromIRBlob(
            session,
            "testModule",
            "test.slang",
            nullptr,
            testDataSize,
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);

        // Test with zero size
        module = slang_loadModuleFromIRBlob(
            session,
            "testModule",
            "test.slang",
            testData,
            0,
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);

        // Test loadModuleInfoFromIRBlob with null session
        SlangInt moduleVersion;
        const char* moduleCompilerVersion;
        const char* moduleName;

        SlangResult infoResult = slang_loadModuleInfoFromIRBlob(
            nullptr,
            testData,
            testDataSize,
            moduleVersion,
            moduleCompilerVersion,
            moduleName);

        SLANG_CHECK(infoResult == SLANG_E_INVALID_ARG);

        // Test loadModuleInfoFromIRBlob with null source
        infoResult = slang_loadModuleInfoFromIRBlob(
            session,
            nullptr,
            testDataSize,
            moduleVersion,
            moduleCompilerVersion,
            moduleName);

        SLANG_CHECK(infoResult == SLANG_E_INVALID_ARG);

        // Test loadModuleInfoFromIRBlob with zero size
        infoResult = slang_loadModuleInfoFromIRBlob(
            session,
            testData,
            0,
            moduleVersion,
            moduleCompilerVersion,
            moduleName);

        SLANG_CHECK(infoResult == SLANG_E_INVALID_ARG);
    }

    // #12852: a null `source` with a valid `path` loads the module from that
    // file; an unreadable `path` diagnoses rather than asserts.
    {
        const char* fileSource = R"(
            [shader("compute")][numthreads(1,1,1)]
            void fileMain() {}
        )";

        ComPtr<ISlangFileSystemExt> fileSystem =
            ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
        auto& memoryFileSystem = *static_cast<MemoryFileSystem*>(fileSystem.get());
        SLANG_CHECK(
            memoryFileSystem.saveFile("from-file.slang", fileSource, strlen(fileSource)) ==
            SLANG_OK);

        slang::SessionDesc fileSessionDesc = sessionDesc;
        fileSessionDesc.fileSystem = fileSystem;
        ComPtr<slang::ISession> fileSession;
        SLANG_CHECK(
            globalSession->createSession(fileSessionDesc, fileSession.writeRef()) == SLANG_OK);

        {
            ComPtr<slang::IModule> module;
            ComPtr<ISlangBlob> diagnostics;
            module = fileSession->loadModuleFromSource(
                "fileModule",
                "from-file.slang",
                nullptr,
                diagnostics.writeRef());
            SLANG_CHECK(module != nullptr);
            if (diagnostics)
            {
                const char* diagText = (const char*)diagnostics->getBufferPointer();
                SLANG_CHECK(strstr(diagText, "error") == nullptr);
            }
        }

        {
            ComPtr<slang::IModule> module;
            ComPtr<ISlangBlob> diagnostics;
            module = fileSession->loadModuleFromSource(
                "missingModule",
                "does-not-exist.slang",
                nullptr,
                diagnostics.writeRef());
            SLANG_CHECK(module == nullptr);
            SLANG_CHECK(diagnostics != nullptr);
            const char* diagText = diagnostics ? (const char*)diagnostics->getBufferPointer() : "";
            SLANG_CHECK(strstr(diagText, "does-not-exist.slang") != nullptr);
        }
    }

    // #12852: the null-blob guard must not assert on the COM entry points the C
    // wrappers reject early — a null IR blob and a null `path` both return null
    // with a diagnostic.
    {
        ComPtr<slang::IModule> module;

        ComPtr<ISlangBlob> irDiagnostics;
        module = session->loadModuleFromIRBlob(
            "irNullBlob",
            "ir.slang",
            nullptr,
            irDiagnostics.writeRef());
        SLANG_CHECK(module == nullptr);
        SLANG_CHECK(irDiagnostics != nullptr);

        ComPtr<ISlangBlob> noPathDiagnostics;
        module =
            session->loadModuleFromSource("noPath", nullptr, nullptr, noPathDiagnostics.writeRef());
        SLANG_CHECK(module == nullptr);
        SLANG_CHECK(noPathDiagnostics != nullptr);
    }
}
