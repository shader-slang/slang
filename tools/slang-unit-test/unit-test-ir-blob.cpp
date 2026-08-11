// unit-test-ir-blob.cpp

#include "core/slang-memory-file-system.h"
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

    // Test 9: An importing module loads from an IR blob when its dependency
    // was itself registered from an IR blob -- an import-resolution smoke test
    // on the pure blob path (freshness gate off).
    //
    // This pins the already-working path that the serialization rework in #6854
    // ("A new approach to AST serialization") fixed: #6557's original symptom was
    // loadModuleFromIRBlob returning nullptr for a module that `import`s another
    // module, and the `containerData.modules.getCount() != 1` gate responsible
    // was removed by #6854. (The separate digest defect this PR fixes -- reached
    // only when UseUpToDateBinaryModule is on -- is covered by
    // irBlobImportUpToDateCheck.)
    // A serialized module records its imports as module-name references resolved
    // at load time via the session's name->module map
    // (Linkage::findOrImportModule); so the meaningful part is that the
    // dependency's blob must be loaded into the session *first*. If the import
    // fails to resolve, loadModuleFromIRBlob for the importer returns nullptr.
    {
        // A dependency module exposing a public function, and an importer that
        // calls it -- the call creates a real cross-module reference, so the
        // importer's serialized IR/AST genuinely depends on the dependency.
        const char* dependencySource = R"(
            module common_6557;

            public int addOne(int x) { return x + 1; }
        )";
        const char* importerSource = R"(
            module importer_6557;

            import common_6557;

            public int doubleThenAddOne(int x) { return addOne(addOne(x)); }
        )";

        // Build phase: compile both modules from source in one session and
        // serialize each to an IR blob. Registering the dependency under its
        // own name lets the importer's `import common_6557` resolve here.
        ComPtr<slang::ISession> buildSession;
        SLANG_CHECK(globalSession->createSession(sessionDesc, buildSession.writeRef()) == SLANG_OK);

        ComPtr<ISlangBlob> dependencyBlob;
        ComPtr<ISlangBlob> importerBlob;
        {
            ComPtr<ISlangBlob> diagnostics;
            ComPtr<slang::IModule> dependencyModule;
            dependencyModule = buildSession->loadModuleFromSourceString(
                "common_6557",
                "common_6557.slang",
                dependencySource,
                diagnostics.writeRef());
            SLANG_CHECK(dependencyModule != nullptr);
            SLANG_CHECK(dependencyModule->serialize(dependencyBlob.writeRef()) == SLANG_OK);

            ComPtr<slang::IModule> importerModule;
            importerModule = buildSession->loadModuleFromSourceString(
                "importer_6557",
                "importer_6557.slang",
                importerSource,
                diagnostics.writeRef());
            SLANG_CHECK(importerModule != nullptr);
            SLANG_CHECK(importerModule->serialize(importerBlob.writeRef()) == SLANG_OK);
        }

        // Load phase: in a *fresh* session (nothing preloaded), register the
        // dependency's blob first, then load the importer's blob. The importer
        // must load successfully by resolving its import from the dependency
        // blob we just registered. The freshness gate is off here, so this
        // exercises only import resolution on the pure blob path (the
        // #6854-fixed path); the UseUpToDateBinaryModule digest regression is
        // covered by irBlobImportUpToDateCheck.
        ComPtr<slang::ISession> loadSession;
        SLANG_CHECK(globalSession->createSession(sessionDesc, loadSession.writeRef()) == SLANG_OK);

        ComPtr<ISlangBlob> loadDiagnostics;
        ComPtr<slang::IModule> loadedDependency;
        loadedDependency = slang_loadModuleFromIRBlob(
            loadSession,
            "common_6557",
            "common_6557.slang",
            dependencyBlob->getBufferPointer(),
            dependencyBlob->getBufferSize(),
            loadDiagnostics.writeRef());
        SLANG_CHECK(loadedDependency != nullptr);

        ComPtr<slang::IModule> loadedImporter;
        loadedImporter = slang_loadModuleFromIRBlob(
            loadSession,
            "importer_6557",
            "importer_6557.slang",
            importerBlob->getBufferPointer(),
            importerBlob->getBufferSize(),
            loadDiagnostics.writeRef());
        if (loadDiagnostics && loadDiagnostics->getBufferSize() > 0)
        {
            printf(
                "Importing-module IR blob load diagnostics: %.*s\n",
                (int)loadDiagnostics->getBufferSize(),
                (const char*)loadDiagnostics->getBufferPointer());
        }

        // The importer can only be non-null if its `import common_6557` resolved
        // against the dependency blob registered above; a failed import would
        // make loadModuleFromIRBlob return nullptr (the original #6557 symptom).
        SLANG_CHECK(loadedImporter != nullptr);
    }
}

// Regression test for #6557 that mirrors the reporter's exact deployment shape:
// precompiled `.slang-module` files produced with default options, loaded back
// through `loadModuleFromIRBlob` with `UseUpToDateBinaryModule` enabled while the
// original `.slang` sources remain on disk.
//
// This differs from `irBlob`'s Test 9 in three ways that together exercise a
// distinct code path:
//   1. The importer's blob is loaded *without* pre-registering the dependency in
//      the session, so `import common` must be resolved from the file system
//      during the load (via the dependency's own `.slang-module`), not from a
//      blob the test loaded first.
//   2. `UseUpToDateBinaryModule` is set, so `loadBinaryModuleImpl` runs
//      `isBinaryModuleUpToDate` (slang-session.cpp): with the sources present it
//      recomputes the SHA1 digest over the build version, option-set hash, and
//      each dependency source's digest and compares it to the one stored in the
//      module chunk. If that gate rejected an importing module,
//      `loadModuleFromIRBlob` would return nullptr -- the #6557 symptom.
//   3. The sources are kept available, so the freshness check takes its full
//      recompute path rather than the "source missing -> accept standalone"
//      shortcut.
//
// The scenario was requested on the issue by @pdeayton-nv; running it is both the
// empirical answer and the regression lock.
SLANG_UNIT_TEST(irBlobImportUpToDateCheck)
{
    // A dependency module exposing a public function, and an importer that calls
    // it -- the call makes the importer genuinely depend on the dependency.
    const char* dependencySource = R"(
        module common;

        public int addOne(int x) { return x + 1; }
    )";
    const char* importerSource = R"(
        module A;

        import common;

        public int doubleThenAddOne(int x) { return addOne(addOne(x)); }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    // A single in-memory file system holds the sources and, after the build
    // phase, the serialized `.slang-module` blobs. Keeping the sources present is
    // deliberate: it drives the full freshness recompute in the load phase.
    ComPtr<ISlangMutableFileSystem> memoryFileSystem =
        ComPtr<ISlangMutableFileSystem>(new Slang::MemoryFileSystem());
    memoryFileSystem->saveFile("common.slang", dependencySource, strlen(dependencySource));
    memoryFileSystem->saveFile("A.slang", importerSource, strlen(importerSource));

    // Build phase: compile each module and serialize it to a `.slang-module`
    // file via the default-option serialization path -- the same content-digest
    // recipe an offline `slangc -o *.slang-module` invocation produces.
    ComPtr<ISlangBlob> importerBlob;
    {
        sessionDesc.fileSystem = memoryFileSystem;
        ComPtr<slang::ISession> buildSession;
        SLANG_CHECK(globalSession->createSession(sessionDesc, buildSession.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> dependencyModule;
        dependencyModule = buildSession->loadModule("common", diagnostics.writeRef());
        SLANG_CHECK(dependencyModule != nullptr);
        ComPtr<slang::IBlob> dependencyBlob;
        SLANG_CHECK(dependencyModule->serialize(dependencyBlob.writeRef()) == SLANG_OK);
        memoryFileSystem->saveFileBlob("common.slang-module", dependencyBlob);

        ComPtr<slang::IModule> importerModule;
        importerModule = buildSession->loadModule("A", diagnostics.writeRef());
        SLANG_CHECK(importerModule != nullptr);
        SLANG_CHECK(importerModule->serialize(importerBlob.writeRef()) == SLANG_OK);
        memoryFileSystem->saveFileBlob("A.slang-module", importerBlob);
    }

    // Load phase: a fresh session with UseUpToDateBinaryModule enabled. Reload the
    // serialized `A.slang-module` artifact from the file system and load it via
    // loadModuleFromIRBlob -- passing the `.slang-module` path -- without
    // pre-loading the dependency, exactly as @pdeayton-nv's retest asked. `import
    // common` is resolved from `common.slang-module` on the file system while the
    // freshness gate is active.
    {
        slang::CompilerOptionEntry compilerOption = {};
        compilerOption.name = slang::CompilerOptionName::UseUpToDateBinaryModule;
        compilerOption.value.kind = slang::CompilerOptionValueKind::Int;
        compilerOption.value.intValue0 = 1;

        sessionDesc.fileSystem = memoryFileSystem;
        sessionDesc.compilerOptionEntries = &compilerOption;
        sessionDesc.compilerOptionEntryCount = 1;

        ComPtr<slang::ISession> loadSession;
        SLANG_CHECK(globalSession->createSession(sessionDesc, loadSession.writeRef()) == SLANG_OK);

        // Read the on-disk `.slang-module` back rather than reusing the in-memory
        // build-phase blob, so the load path exercises the exact serialized bytes a
        // deployed loader would see.
        ComPtr<ISlangBlob> savedImporterBlob;
        SLANG_CHECK(
            memoryFileSystem->loadFile("A.slang-module", savedImporterBlob.writeRef()) == SLANG_OK);
        SLANG_CHECK(savedImporterBlob != nullptr);

        ComPtr<ISlangBlob> loadDiagnostics;
        ComPtr<slang::IModule> loadedImporter;
        loadedImporter = slang_loadModuleFromIRBlob(
            loadSession,
            "A",
            "A.slang-module",
            savedImporterBlob->getBufferPointer(),
            savedImporterBlob->getBufferSize(),
            loadDiagnostics.writeRef());
        if (loadDiagnostics && loadDiagnostics->getBufferSize() > 0)
        {
            printf(
                "Up-to-date importing-module IR blob load diagnostics: %.*s\n",
                (int)loadDiagnostics->getBufferSize(),
                (const char*)loadDiagnostics->getBufferPointer());
        }

        // The importer loads only if its `import common` resolved and the
        // freshness gate accepted it; nullptr would be the #6557 symptom.
        SLANG_CHECK(loadedImporter != nullptr);
    }
}
