// unit-test-precompiled-module-attribute.cpp

// Test loading a precompiled module that defines a user defined attribute.

#include "core/slang-io.h"
#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test loading a precompiled module that imports other modules.
//
// This test checks a nuanced case:
// Given module `test` that imports module `foo`, where `foo` defines a struct type with
// [__AttributeUsage] to make it a user defined attribute, but `foo` itself does not make
// any use of that attribute.
// Then `test` contains decls using the user defined attribute from `foo`.
// Now, if we precompile `test` into a module, but leave `foo` as source, and then try
// to load the precompiled `test` module, we had a bug where the user defined attribute
// cannot be found from the imported `foo` module, leading to an error.
//
// The reason this was happening is that for every struct decl with [__AttributeUsage], we
// will synthesize a `AttributeDecl` representing the attribute, and the AttributeDecl, instead
// of the defining struct type decl is what gets referenced from any use sites of the attribute.
// However, the synthesis of the AttributeDecl was happening on-demand when we lookup an attribute
// from the [] bracket syntax on use sites. Therefore, when we serialize `foo` itself, since it
// does not use the attribute itself, the AttributeDecl was never synthesized and serialized.
// Now when we load the precompiled `test` module, it expects this AttributeDecl to be present in
// `foo`, but it is not, leading to the error.
//
// This issue has been fixed by ensuring that AttributeDecls are always synthesized actively upon
// checking of [__AttributeUsage] on struct decls, regardless of whether the attribute
// is used in the defining module or not.
//
SLANG_UNIT_TEST(precompiledModuleWithUserDefinedAttribute)
{
    // Prepare the virtual file system with the module files.
    const char* fooSource = R"(
        module foo;
        [__AttributeUsage(_AttributeTargets.Var)]
        public struct TextureAttribute
        {
            string name;
        };
    )";
    const char* testSource = R"(
        module test;
        import foo;

        [Texture("myTextureName")]
        RWTexture2D texture;

        [numthreads(1, 1, 1)]
        void computeMain(uint3 idx: SV_DispatchThreadID)
        {
            texture[idx.xy] = float4(1, 0, 0, 1);
        }
    )";
    ComPtr<ISlangFileSystemExt> fs = ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
    auto& memoryFS = *static_cast<MemoryFileSystem*>(fs.get());
    memoryFS.saveFile("test.slang", testSource, strlen(testSource));
    memoryFS.saveFile("foo.slang", fooSource, strlen(fooSource));

    // `precompiledFs` will hold `test.slang-module` (precompiled) and `foo.slang` (source).
    ComPtr<ISlangFileSystemExt> precompiledFs = ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
    auto& memoryPrecompiledFS = *static_cast<MemoryFileSystem*>(precompiledFs.get());

    memoryPrecompiledFS.saveFile("foo.slang", fooSource, strlen(fooSource));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    sessionDesc.targets = &targetDesc;

    // Phase 1: load debug module and save precompiled IR modules.
    {
        ComPtr<slang::ISession> session;
        sessionDesc.fileSystem = fs;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangBlob> diagnostics;

        auto testModule = session->loadModule("test", diagnostics.writeRef());

        SLANG_CHECK_ABORT(testModule != nullptr);
        if (diagnostics)
        {
            // If there are diagnostics, they should be warnings or errors
            SLANG_CHECK_ABORT(diagnostics->getBufferSize() > 0);
        }

        for (int i = 0; i < session->getLoadedModuleCount(); i++)
        {
            slang::IModule* mod = session->getLoadedModule(i);

            String path = (StringBuilder() << mod->getFilePath() << "-module");
            if (path == "test.slang-module")
            {
                // Save only `test.slang-module` as precompiled module.
                ComPtr<slang::IBlob> serializedBlob;
                SLANG_CHECK_ABORT(mod->serialize(serializedBlob.writeRef()) == SLANG_OK);
                memoryPrecompiledFS.saveFileBlob(path.getBuffer(), serializedBlob);
            }
        }
    }

    // Phase 2: Create a new session and load from precompiled modules.
    {
        ComPtr<slang::ISession> session;
        sessionDesc.fileSystem = precompiledFs;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangBlob> diagnostics;
        auto module = session->loadModule("test", diagnostics.writeRef());
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
}
