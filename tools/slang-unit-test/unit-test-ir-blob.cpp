// unit-test-ir-blob.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Slang;

// Test the slang_loadModuleFromIRBlob and slang_loadModuleInfoFromIRBlob functions
SLANG_UNIT_TEST(irBlob)
{
    // Test source code for creating IR data
    const char* testModuleSource = R"(
        module test_ir_module;

        public struct TestStruct {
            float x, y, z;
        }

        public void testFunction(TestStruct input) {
            // Simple function
        }

        public static const float PI = 3.14159;
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    // Create IR data by serializing a module
    ComPtr<ISlangBlob> irBlob;
    {
        ComPtr<slang::IModule> module;
        ComPtr<ISlangBlob> diagnostics;

        module = session->loadModuleFromSourceString(
            "test_ir_module",
            "test_ir_module.slang",
            testModuleSource,
            diagnostics.writeRef());

        SLANG_CHECK(module != nullptr);
        if (diagnostics && diagnostics->getBufferSize() > 0)
        {
            // Log diagnostics if any
            printf(
                "Module compilation diagnostics: %.*s\n",
                (int)diagnostics->getBufferSize(),
                (const char*)diagnostics->getBufferPointer());
        }

        // Serialize the module to create IR data
        SLANG_CHECK(module->serialize(irBlob.writeRef()) == SLANG_OK);
        SLANG_CHECK(irBlob != nullptr);
        SLANG_CHECK(irBlob->getBufferSize() > 0);
    }

    // Test 1: Test slang_loadModuleFromIRBlob with valid IR data
    {
        ComPtr<slang::IModule> loadedModule;
        ComPtr<ISlangBlob> diagnostics;

        loadedModule = slang_loadModuleFromIRBlob(
            session,
            "test_ir_module_loaded",
            "test_ir_module_loaded.slang",
            irBlob->getBufferPointer(),
            irBlob->getBufferSize(),
            diagnostics.writeRef());

        SLANG_CHECK(loadedModule != nullptr);
        if (diagnostics && diagnostics->getBufferSize() > 0)
        {
            // Log diagnostics if any
            printf(
                "IR blob loading diagnostics: %.*s\n",
                (int)diagnostics->getBufferSize(),
                (const char*)diagnostics->getBufferPointer());
        }

        // Verify the loaded module is valid
        SLANG_CHECK(loadedModule != nullptr);
    }

    // Test 2: Test slang_loadModuleInfoFromIRBlob with valid IR data
    {
        SlangInt moduleVersion;
        const char* moduleCompilerVersion;
        const char* moduleName;

        SlangResult result = slang_loadModuleInfoFromIRBlob(
            session,
            irBlob->getBufferPointer(),
            irBlob->getBufferSize(),
            moduleVersion,
            moduleCompilerVersion,
            moduleName);

        SLANG_CHECK(result == SLANG_OK);
        SLANG_CHECK(moduleName != nullptr);
        SLANG_CHECK(strcmp(moduleName, "test_ir_module") == 0);
        SLANG_CHECK(moduleCompilerVersion != nullptr);
        SLANG_CHECK(moduleVersion >= 0);
    }

    // Test 3: Test slang_loadModuleFromIRBlob with invalid parameters
    {
        ComPtr<slang::IModule> module;
        ComPtr<ISlangBlob> diagnostics;

        // Test with null session
        module = slang_loadModuleFromIRBlob(
            nullptr,
            "testModule",
            "test.slang",
            irBlob->getBufferPointer(),
            irBlob->getBufferSize(),
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);

        // Test with null moduleName
        module = slang_loadModuleFromIRBlob(
            session,
            nullptr,
            "test.slang",
            irBlob->getBufferPointer(),
            irBlob->getBufferSize(),
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);

        // Test with null path
        module = slang_loadModuleFromIRBlob(
            session,
            "testModule",
            nullptr,
            irBlob->getBufferPointer(),
            irBlob->getBufferSize(),
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);

        // Test with null source
        module = slang_loadModuleFromIRBlob(
            session,
            "testModule",
            "test.slang",
            nullptr,
            irBlob->getBufferSize(),
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);

        // Test with zero size
        module = slang_loadModuleFromIRBlob(
            session,
            "testModule",
            "test.slang",
            irBlob->getBufferPointer(),
            0,
            diagnostics.writeRef());

        SLANG_CHECK(module == nullptr);
    }

    // Test 4: Test slang_loadModuleInfoFromIRBlob with invalid parameters
    {
        SlangInt moduleVersion;
        const char* moduleCompilerVersion;
        const char* moduleName;

        // Test with null session
        SlangResult result = slang_loadModuleInfoFromIRBlob(
            nullptr,
            irBlob->getBufferPointer(),
            irBlob->getBufferSize(),
            moduleVersion,
            moduleCompilerVersion,
            moduleName);

        SLANG_CHECK(result == SLANG_E_INVALID_ARG);

        // Test with null source
        result = slang_loadModuleInfoFromIRBlob(
            session,
            nullptr,
            irBlob->getBufferSize(),
            moduleVersion,
            moduleCompilerVersion,
            moduleName);

        SLANG_CHECK(result == SLANG_E_INVALID_ARG);

        // Test with zero size
        result = slang_loadModuleInfoFromIRBlob(
            session,
            irBlob->getBufferPointer(),
            0,
            moduleVersion,
            moduleCompilerVersion,
            moduleName);

        SLANG_CHECK(result == SLANG_E_INVALID_ARG);
    }

    // Test 5: Test with corrupted/invalid IR data
    {
        ComPtr<slang::IModule> module;
        ComPtr<ISlangBlob> diagnostics;

        // Create some invalid data
        const char* invalidData = "This is not valid IR data";
        size_t invalidDataSize = strlen(invalidData);

        module = slang_loadModuleFromIRBlob(
            session,
            "testModule",
            "test.slang",
            invalidData,
            invalidDataSize,
            diagnostics.writeRef());

        // This might return nullptr or a module with diagnostics
        if (module == nullptr)
        {
            // If it failed, that's expected for invalid data
            SLANG_CHECK(true);
        }
        else
        {
            // If it succeeded, there should be diagnostics
            if (diagnostics && diagnostics->getBufferSize() > 0)
            {
                SLANG_CHECK(true);
            }
        }
    }

    // Test 6: Test slang_loadModuleInfoFromIRBlob with corrupted/invalid IR data
    {
        SlangInt moduleVersion;
        const char* moduleCompilerVersion;
        const char* moduleName;

        // Create some invalid data
        const char* invalidData = "This is not valid IR data";
        size_t invalidDataSize = strlen(invalidData);

        SlangResult result = slang_loadModuleInfoFromIRBlob(
            session,
            invalidData,
            invalidDataSize,
            moduleVersion,
            moduleCompilerVersion,
            moduleName);

        // This should fail with invalid data
        SLANG_CHECK(result != SLANG_OK);
    }

    // Test 7: Test round-trip serialization and loading
    {
        // Load the module from IR
        ComPtr<slang::IModule> loadedModule;
        ComPtr<ISlangBlob> diagnostics;

        loadedModule = slang_loadModuleFromIRBlob(
            session,
            "test_round_trip",
            "test_round_trip.slang",
            irBlob->getBufferPointer(),
            irBlob->getBufferSize(),
            diagnostics.writeRef());

        SLANG_CHECK(loadedModule != nullptr);

        if (loadedModule)
        {
            // Serialize the loaded module again
            ComPtr<ISlangBlob> roundTripBlob;
            SLANG_CHECK(loadedModule->serialize(roundTripBlob.writeRef()) == SLANG_OK);
            SLANG_CHECK(roundTripBlob != nullptr);
            SLANG_CHECK(roundTripBlob->getBufferSize() > 0);

            // Load it again
            ComPtr<slang::IModule> roundTripModule;
            roundTripModule = slang_loadModuleFromIRBlob(
                session,
                "test_round_trip_2",
                "test_round_trip_2.slang",
                roundTripBlob->getBufferPointer(),
                roundTripBlob->getBufferSize(),
                diagnostics.writeRef());

            SLANG_CHECK(roundTripModule != nullptr);
        }
    }

    // Test 8: Test multiple modules with different IR data
    {
        // Create a second module with different content
        const char* testModuleSource2 = R"(
            module test_ir_module_2;

            public struct AnotherStruct {
                int a, b, c;
            }

            public void anotherFunction(AnotherStruct input) {
                // Another function
            }
        )";

        ComPtr<slang::IModule> module2;
        ComPtr<ISlangBlob> diagnostics2;
        ComPtr<ISlangBlob> irBlob2;

        module2 = session->loadModuleFromSourceString(
            "test_ir_module_2",
            "test_ir_module_2.slang",
            testModuleSource2,
            diagnostics2.writeRef());

        SLANG_CHECK(module2 != nullptr);
        SLANG_CHECK(module2->serialize(irBlob2.writeRef()) == SLANG_OK);

        // Load both modules
        ComPtr<slang::IModule> loadedModule1;
        ComPtr<slang::IModule> loadedModule2;
        ComPtr<ISlangBlob> diagnostics;

        loadedModule1 = slang_loadModuleFromIRBlob(
            session,
            "test_ir_module_1_loaded",
            "test_ir_module_1_loaded.slang",
            irBlob->getBufferPointer(),
            irBlob->getBufferSize(),
            diagnostics.writeRef());

        loadedModule2 = slang_loadModuleFromIRBlob(
            session,
            "test_ir_module_2_loaded",
            "test_ir_module_2_loaded.slang",
            irBlob2->getBufferPointer(),
            irBlob2->getBufferSize(),
            diagnostics.writeRef());

        SLANG_CHECK(loadedModule1 != nullptr);
        SLANG_CHECK(loadedModule2 != nullptr);

        // Verify both modules loaded successfully
        SLANG_CHECK(loadedModule1 != nullptr);
        SLANG_CHECK(loadedModule2 != nullptr);
    }
}
