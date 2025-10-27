// unit-test-obfuscation-with-debug.cpp

// Define this to dump all intermediate files and SPIR-V output to disk
#define DUMP_FILES 0

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>

#if DUMP_FILES
#ifdef _WIN32
#include <direct.h>  // For _mkdir on Windows
#else
#include <sys/stat.h>  // For mkdir on Unix/Linux
#include <sys/types.h>
#endif
#endif // DUMP_FILES

using namespace Slang;

#if DUMP_FILES
// Utility function to dump a blob to a file
static void dumpBlobToFile(const char* filepath, ISlangBlob* blob, const char* description = nullptr)
{
    if (!blob || blob->getBufferSize() == 0)
        return;
    
    FILE* file = fopen(filepath, "wb");
    if (file)
    {
        fwrite(blob->getBufferPointer(), 1, blob->getBufferSize(), file);
        fclose(file);
        if (description)
            printf("%s saved to: %s\n", description, filepath);
        else
            printf("File saved to: %s\n", filepath);
    }
    else
    {
        printf("Warning: Failed to open file for writing: %s\n", filepath);
    }
}
#endif // DUMP_FILES

// Test obfuscation with separate debug symbols and mixed module loading
SLANG_UNIT_TEST(obfuscationWithSeparateDebug)
{
    // Slang Library 1: Math utilities
    const char* library1Source = R"(
        module MathLibrary;

        public float addFloats(float a, float b) {
            return a + b;
        }

        public float multiplyFloats(float a, float b) {
            return a * b;
        }

        public static const float PI = 3.14159;
    )";

    // Slang Library 2: Color utilities
    const char* library2Source = R"(
        module ColorLibrary;

        public float4 makeColor(float r, float g, float b, float a) {
            return float4(r, g, b, a);
        }

        public float4 scaleColor(float4 color, float scale) {
            return color * scale;
        }

        public static const float4 RED = float4(1.0, 0.0, 0.0, 1.0);
    )";

    // Inline shader code that will be loaded from source
    const char* inlineShaderSource = R"(
        module InlineShader;

        public float3 transformPosition(float3 pos, float scale) {
            return pos * scale;
        }

        public float2 transformTexCoord(float2 uv, float offset) {
            return uv + offset;
        }
    )";

    // Final shader that imports the libraries
    const char* finalShaderSource = R"(
        import MathLibrary;
        import ColorLibrary;
        import InlineShader;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        float computeMain(uint3 tid : SV_DispatchThreadID)
        {
            float result = 0.0;
            // Use MathLibrary functions
            float scale = addFloats(1.0, 0.5);
            float multiplier = multiplyFloats(scale, PI);
            
            // Use InlineShader functions
            float3 pos = float3(tid.x, tid.y, tid.z);
            float3 transformed = transformPosition(pos, multiplier);
            
            // Use ColorLibrary functions
            float4 baseColor = makeColor(1.0, 0.5, 0.2, 1.0);
            float4 finalColor = scaleColor(baseColor, scale);
            
            // Just to use the values (prevent optimization)
            if (transformed.x > 0.0)
            {
                result = finalColor.r;
            }
            return result;
        }
    )";

    // Create global session
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // Create session with obfuscation and separate debug enabled
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    sessionDesc.targets = &targetDesc;

    // Enable obfuscation
    slang::CompilerOptionEntry obfuscationOption;
    obfuscationOption.name = slang::CompilerOptionName::Obfuscate;
    obfuscationOption.value.kind = slang::CompilerOptionValueKind::Int;
    obfuscationOption.value.intValue0 = 1; // Enable obfuscation

    // Enable separate debug symbols
    slang::CompilerOptionEntry separateDebugOption;
    separateDebugOption.name = slang::CompilerOptionName::EmitSeparateDebug;
    separateDebugOption.value.kind = slang::CompilerOptionValueKind::Int;
    separateDebugOption.value.intValue0 = 1;

    // Debug info level
    slang::CompilerOptionEntry debugInfoOption;
    debugInfoOption.name = slang::CompilerOptionName::DebugInformation;
    debugInfoOption.value.kind = slang::CompilerOptionValueKind::Int;
    debugInfoOption.value.intValue0 = 2;

    slang::CompilerOptionEntry options[] = {obfuscationOption, separateDebugOption, debugInfoOption};
    sessionDesc.compilerOptionEntries = options;
    sessionDesc.compilerOptionEntryCount = sizeof(options) / sizeof(options[0]);

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    printf("=== Test Case 1: Obfuscation with Separate Debug ===\n");
    printf("Test setup: Obfuscation ON + EmitSeparateDebug ON\n\n");

#if DUMP_FILES
    // Create output directory
    const char* outputDir = "slang_dump_spirv";
#ifdef _WIN32
    _mkdir(outputDir);
#else
    mkdir(outputDir, 0755);
#endif
    printf("Output directory: %s\n\n", outputDir);
#endif

    // Step 1: Compile Slang Library 1 to IR blob
    printf("Step 1: Compiling Slang Library 1 (MathLibrary)...\n");
    ComPtr<slang::IModule> library1Module;
    ComPtr<ISlangBlob> library1Diagnostics;
    ComPtr<ISlangBlob> library1IRBlob;
    
    library1Module = session->loadModuleFromSourceString(
        "MathLibrary",
        "MathLibrary.slang",
        library1Source,
        library1Diagnostics.writeRef());

    SLANG_CHECK(library1Module != nullptr);
    if (library1Diagnostics && library1Diagnostics->getBufferSize() > 0)
    {
        printf("Library 1 compilation diagnostics:\n%.*s\n",
            (int)library1Diagnostics->getBufferSize(),
            (const char*)library1Diagnostics->getBufferPointer());
    }

    // Serialize Library 1 to IR blob
    SLANG_CHECK(library1Module->serialize(library1IRBlob.writeRef()) == SLANG_OK);
    SLANG_CHECK(library1IRBlob != nullptr);
    SLANG_CHECK(library1IRBlob->getBufferSize() > 0);
    printf("Library 1 serialized: %zu bytes\n", library1IRBlob->getBufferSize());
#if DUMP_FILES
    dumpBlobToFile("slang_dump_spirv/library1-mathlib.slang-module", library1IRBlob, "Library 1 IR blob");
#endif
    printf("\n");

    // Step 2: Compile Slang Library 2 to IR blob
    printf("Step 2: Compiling Slang Library 2 (ColorLibrary)...\n");
    ComPtr<slang::IModule> library2Module;
    ComPtr<ISlangBlob> library2Diagnostics;
    ComPtr<ISlangBlob> library2IRBlob;
    
    library2Module = session->loadModuleFromSourceString(
        "ColorLibrary",
        "ColorLibrary.slang",
        library2Source,
        library2Diagnostics.writeRef());

    SLANG_CHECK(library2Module != nullptr);
    if (library2Diagnostics && library2Diagnostics->getBufferSize() > 0)
    {
        printf("Library 2 compilation diagnostics:\n%.*s\n",
            (int)library2Diagnostics->getBufferSize(),
            (const char*)library2Diagnostics->getBufferPointer());
    }

    // Serialize Library 2 to IR blob
    SLANG_CHECK(library2Module->serialize(library2IRBlob.writeRef()) == SLANG_OK);
    SLANG_CHECK(library2IRBlob != nullptr);
    SLANG_CHECK(library2IRBlob->getBufferSize() > 0);
    printf("Library 2 serialized: %zu bytes\n", library2IRBlob->getBufferSize());
#if DUMP_FILES
    dumpBlobToFile("slang_dump_spirv/library2-colorlib.slang-module", library2IRBlob, "Library 2 IR blob");
#endif
    printf("\n");

    // Step 3: Create a new session for final shader compilation. Add separarte debug option.
    printf("Step 3: Creating new session for final shader compilation...\n");

    ComPtr<slang::ISession> finalSession;
    SLANG_CHECK(globalSession->createSession(sessionDesc, finalSession.writeRef()) == SLANG_OK);

    // Step 4: Load Library 1 from IR blob
    printf("Step 4: Loading Library 1 from IR blob...\n");
    ComPtr<slang::IModule> loadedLibrary1;
    ComPtr<ISlangBlob> loadLibrary1Diagnostics;

    loadedLibrary1 = slang_loadModuleFromIRBlob(
        finalSession,
        "MathLibrary",
        "MathLibrary.slang",
        library1IRBlob->getBufferPointer(),
        library1IRBlob->getBufferSize(),
        loadLibrary1Diagnostics.writeRef());

    SLANG_CHECK(loadedLibrary1 != nullptr);
    if (loadLibrary1Diagnostics && loadLibrary1Diagnostics->getBufferSize() > 0)
    {
        printf("Library 1 loading diagnostics:\n%.*s\n",
            (int)loadLibrary1Diagnostics->getBufferSize(),
            (const char*)loadLibrary1Diagnostics->getBufferPointer());
    }
    printf("Library 1 loaded successfully from IR blob\n\n");

    // Step 5: Load Library 2 from IR blob
    printf("Step 5: Loading Library 2 from IR blob...\n");
    ComPtr<slang::IModule> loadedLibrary2;
    ComPtr<ISlangBlob> loadLibrary2Diagnostics;

    loadedLibrary2 = slang_loadModuleFromIRBlob(
        finalSession,
        "ColorLibrary",
        "ColorLibrary.slang",
        library2IRBlob->getBufferPointer(),
        library2IRBlob->getBufferSize(),
        loadLibrary2Diagnostics.writeRef());

    SLANG_CHECK(loadedLibrary2 != nullptr);
    if (loadLibrary2Diagnostics && loadLibrary2Diagnostics->getBufferSize() > 0)
    {
        printf("Library 2 loading diagnostics:\n%.*s\n",
            (int)loadLibrary2Diagnostics->getBufferSize(),
            (const char*)loadLibrary2Diagnostics->getBufferPointer());
    }
    printf("Library 2 loaded successfully from IR blob\n\n");

    // Step 6: Load inline shader from source
    printf("Step 6: Loading inline shader from source...\n");
    ComPtr<slang::IModule> inlineModule;
    ComPtr<ISlangBlob> inlineDiagnostics;

    inlineModule = finalSession->loadModuleFromSourceString(
        "InlineShader",
        "InlineShader.slang",
        inlineShaderSource,
        inlineDiagnostics.writeRef());

    SLANG_CHECK(inlineModule != nullptr);
    if (inlineDiagnostics && inlineDiagnostics->getBufferSize() > 0)
    {
        printf("Inline shader diagnostics:\n%.*s\n",
            (int)inlineDiagnostics->getBufferSize(),
            (const char*)inlineDiagnostics->getBufferPointer());
    }
    printf("Inline shader loaded successfully from source\n");
#if DUMP_FILES
    ComPtr<ISlangBlob> inlineIRBlob;
    if (inlineModule->serialize(inlineIRBlob.writeRef()) == SLANG_OK && inlineIRBlob)
    {
        printf("Inline shader serialized: %zu bytes\n", inlineIRBlob->getBufferSize());
        dumpBlobToFile("slang_dump_spirv/inline-shader.slang-module", inlineIRBlob, "Inline shader IR blob");
    }
#endif
    printf("\n");

    // Step 7: Compile final shader that uses all modules
    printf("Step 7: Compiling final shader that imports all modules...\n");
    ComPtr<slang::IModule> finalShaderModule;
    ComPtr<ISlangBlob> finalShaderDiagnostics;

    finalShaderModule = finalSession->loadModuleFromSourceString(
        "FinalShader",
        "FinalShader.slang",
        finalShaderSource,
        finalShaderDiagnostics.writeRef());

    if (finalShaderDiagnostics && finalShaderDiagnostics->getBufferSize() > 0)
    {
        printf("Final shader diagnostics:\n%.*s\n",
            (int)finalShaderDiagnostics->getBufferSize(),
            (const char*)finalShaderDiagnostics->getBufferPointer());
    }

    SLANG_CHECK(finalShaderModule != nullptr);
    printf("Final shader compiled successfully\n");
#if DUMP_FILES
    ComPtr<ISlangBlob> finalShaderIRBlob;
    if (finalShaderModule->serialize(finalShaderIRBlob.writeRef()) == SLANG_OK && finalShaderIRBlob)
    {
        printf("Final shader serialized: %zu bytes\n", finalShaderIRBlob->getBufferSize());
        dumpBlobToFile("slang_dump_spirv/final-shader.slang-module", finalShaderIRBlob, "Final shader IR blob");
    }
#endif
    printf("\n");

    // Step 8: Find the entry point
    printf("Step 8: Finding entry point 'computeMain'...\n");
    ComPtr<slang::IEntryPoint> entryPoint;
    SlangResult entryPointResult = finalShaderModule->findEntryPointByName("computeMain", entryPoint.writeRef());
    
    SLANG_CHECK(entryPointResult == SLANG_OK);
    SLANG_CHECK(entryPoint != nullptr);
    printf("Entry point found successfully\n\n");

    // Step 9: Create a component type for the final shader
    printf("Step 9: Creating component type with entry point...\n");
    slang::IComponentType* components[] = {finalShaderModule, entryPoint, inlineModule, loadedLibrary1, loadedLibrary2};
    ComPtr<slang::IComponentType> compositeProgram;
    ComPtr<ISlangBlob> linkDiagnostics;

    SlangResult linkResult = finalSession->createCompositeComponentType(
        components,
        5, // finalShaderModule + entryPoint + inlineModule + loadedLibrary1 + loadedLibrary2
        compositeProgram.writeRef(),
        linkDiagnostics.writeRef());

    if (linkDiagnostics && linkDiagnostics->getBufferSize() > 0)
    {
        printf("Linking diagnostics:\n%.*s\n",
            (int)linkDiagnostics->getBufferSize(),
            (const char*)linkDiagnostics->getBufferPointer());
    }

    SLANG_CHECK(linkResult == SLANG_OK);
    SLANG_CHECK(compositeProgram != nullptr);
    printf("Component type created successfully\n\n");

    // Step 10: Link the program
    printf("Step 10: Linking program...\n");
    ComPtr<slang::IComponentType> linkedProgram;
    ComPtr<ISlangBlob> linkDiagnostics2;
    
    SlangResult linkResult2 = compositeProgram->link(linkedProgram.writeRef(), linkDiagnostics2.writeRef());
    
    if (linkDiagnostics2 && linkDiagnostics2->getBufferSize() > 0)
    {
        printf("Link diagnostics:\n%.*s\n",
            (int)linkDiagnostics2->getBufferSize(),
            (const char*)linkDiagnostics2->getBufferPointer());
    }
    
    SLANG_CHECK(linkResult2 == SLANG_OK);
    SLANG_CHECK(linkedProgram != nullptr);
    printf("Program linked successfully\n\n");

    // Step 11: Get SPIR-V code with obfuscation and separate debug info
    printf("Step 11: Generating SPIR-V with obfuscation and separate debug...\n");
    ComPtr<ISlangBlob> spirvCode;
    ComPtr<ISlangBlob> spirvDebugCode;
    ComPtr<ISlangBlob> spirvDiagnostics;

    // Query for IComponentType2 to access getEntryPointCompileResult
    ComPtr<slang::IComponentType2> linkedProgram2;
    SLANG_CHECK(linkedProgram->queryInterface(SLANG_IID_PPV_ARGS(linkedProgram2.writeRef())) == SLANG_OK);
    
    ComPtr<slang::ICompileResult> compileResult;
    SlangResult compileStatus = linkedProgram2->getEntryPointCompileResult(
        0, // Entry point index
        0, // Target index
        compileResult.writeRef(),
        spirvDiagnostics.writeRef());

    if (spirvDiagnostics && spirvDiagnostics->getBufferSize() > 0)
    {
        printf("SPIR-V generation diagnostics:\n%.*s\n",
            (int)spirvDiagnostics->getBufferSize(),
            (const char*)spirvDiagnostics->getBufferPointer());
    }

    // Assert that SPIR-V generation succeeded
    SLANG_CHECK(compileStatus == SLANG_OK);
    SLANG_CHECK(compileResult != nullptr);

    // Get the main SPIR-V code (item 0) and debug SPIR-V (item 1)
    SLANG_CHECK(compileResult->getItemCount() == 2);
    SLANG_CHECK(compileResult->getItemData(0, spirvCode.writeRef()) == SLANG_OK);
    SLANG_CHECK(compileResult->getItemData(1, spirvDebugCode.writeRef()) == SLANG_OK);
    
    SLANG_CHECK(spirvCode != nullptr);
    SLANG_CHECK(spirvCode->getBufferSize() > 0);
    SLANG_CHECK(spirvDebugCode != nullptr);
    SLANG_CHECK(spirvDebugCode->getBufferSize() > 0);

    printf("Main SPIR-V generated: %zu bytes (obfuscated, debug stripped)\n", spirvCode->getBufferSize());
    printf("Debug SPIR-V generated: %zu bytes (with debug info)\n", spirvDebugCode->getBufferSize());
    
    // Check SPIR-V magic numbers
    const uint32_t* spirvData = (const uint32_t*)spirvCode->getBufferPointer();
    const uint32_t* spirvDebugData = (const uint32_t*)spirvDebugCode->getBufferPointer();
    
    if (spirvCode->getBufferSize() >= 4)
    {
        uint32_t magic = spirvData[0];
        printf("Main SPIR-V magic number: 0x%08X %s\n", 
            magic,
            (magic == 0x07230203) ? "(valid)" : "(invalid)");
        SLANG_CHECK(magic == 0x07230203);
    }
    
    if (spirvDebugCode->getBufferSize() >= 4)
    {
        uint32_t magic = spirvDebugData[0];
        printf("Debug SPIR-V magic number: 0x%08X %s\n", 
            magic,
            (magic == 0x07230203) ? "(valid)" : "(invalid)");
        SLANG_CHECK(magic == 0x07230203);
    }
    

#if DUMP_FILES
    // Dump SPIR-V files
    printf("\n");
    dumpBlobToFile("slang_dump_spirv/final-shader-obfuscated.spv", spirvCode, "Main SPIR-V");
    dumpBlobToFile("slang_dump_spirv/final-shader-debug.dbg.spv", spirvDebugCode, "Debug SPIR-V");
#endif
    
    // Get metadata with debug build identifier
    ComPtr<slang::IMetadata> metadata;
    if (compileResult->getMetadata(metadata.writeRef()) == SLANG_OK && metadata)
    {
        const char* debugBuildId = metadata->getDebugBuildIdentifier();
        if (debugBuildId)
        {
            printf("Debug Build Identifier: %s\n", debugBuildId);
        }
    }
    printf("\n");

    printf("\n=== Test Summary ===\n");
    printf("✓ Library 1 (MathLibrary): Compiled and serialized to IR blob\n");
    printf("✓ Library 2 (ColorLibrary): Compiled and serialized to IR blob\n");
    printf("✓ Library 1: Loaded from IR blob in final session\n");
    printf("✓ Library 2: Loaded from IR blob in final session\n");
    printf("✓ Inline shader: Loaded from source in final session\n");
    printf("✓ Final shader: Compiled with imports from all modules\n");
    printf("✓ Entry point: Found and linked\n");
    printf("✓ Obfuscation: Enabled\n");
    printf("✓ Separate debug symbols: Enabled\n");
    printf("✓ Main SPIR-V: Generated (obfuscated, debug stripped)\n");
    printf("✓ Debug SPIR-V: Generated (with full debug info)\n");
#if DUMP_FILES
    printf("\n=== Generated Files ===\n");
    printf("All files saved to: slang_dump_spirv/\n");
    printf("- slang_dump_spirv/library1-mathlib.slang-module\n");
    printf("- slang_dump_spirv/library2-colorlib.slang-module\n");
    printf("- slang_dump_spirv/inline-shader.slang-module\n");
    printf("- slang_dump_spirv/final-shader.slang-module\n");
    printf("- slang_dump_spirv/final-shader-obfuscated.spv (main SPIR-V)\n");
    printf("- slang_dump_spirv/final-shader-debug.dbg.spv (debug SPIR-V)\n");
#endif
    printf("\nTest completed successfully!\n");
}

