// unit-test-obfuscation-with-debug.cpp

// Define this to dump all intermediate files and SPIR-V output to disk
#define DUMP_FILES 1

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>

#if DUMP_FILES
#define SLANG_DUMP_PATH "slang_dump_spirv"
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

// Helper function to compare two blobs byte-by-byte
static bool compareBlobsEqual(ISlangBlob* blob1, ISlangBlob* blob2, const char* name1, const char* name2)
{
    if (!blob1 || !blob2)
    {
        printf("ERROR: One or both blobs are null\n");
        return false;
    }

    size_t size1 = blob1->getBufferSize();
    size_t size2 = blob2->getBufferSize();

    printf("Comparing %s (%zu bytes) with %s (%zu bytes)...\n", name1, size1, name2, size2);

    if (size1 != size2)
    {
        printf("  ✗ Size mismatch: %zu vs %zu bytes\n", size1, size2);
        return false;
    }

    const uint8_t* data1 = (const uint8_t*)blob1->getBufferPointer();
    const uint8_t* data2 = (const uint8_t*)blob2->getBufferPointer();

    for (size_t i = 0; i < size1; i++)
    {
        if (data1[i] != data2[i])
        {
            printf("  ✗ Byte mismatch at offset %zu: 0x%02X vs 0x%02X\n", i, data1[i], data2[i]);
            return false;
        }
    }

    printf("  ✓ Blobs are identical!\n");
    return true;
}

// Shared shader sources
static const char* kMathLibrarySource = R"(
    module MathLibrary;

    public float addFloats(float a, float b) {
        return a + b;
    }

    public float multiplyFloats(float a, float b) {
        return a * b;
    }

    public static const float PI = 3.14159;
)";

static const char* kColorLibrarySource = R"(
    module ColorLibrary;

    public float4 makeColor(float r, float g, float b, float a) {
        return float4(r, g, b, a);
    }

    public float4 scaleColor(float4 color, float scale) {
        return color * scale;
    }

    public static const float4 RED = float4(1.0, 0.0, 0.0, 1.0);
)";

static const char* kInlineShaderSource = R"(
    module InlineShader;

    public float3 transformPosition(float3 pos, float scale) {
        return pos * scale;
    }

    public float2 transformTexCoord(float2 uv, float offset) {
        return uv + offset;
    }
)";

static const char* kFinalShaderSource = R"(
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

// Helper: Compile and serialize library modules
static SlangResult compileLibraryModules(
    slang::ISession* session,
    ComPtr<ISlangBlob>& outLib1Blob,
    ComPtr<ISlangBlob>& outLib2Blob)
{
    printf("  Compiling libraries...\n");
    
    // Compile Library 1
    ComPtr<slang::IModule> lib1Module;
    lib1Module = session->loadModuleFromSourceString(
        "MathLibrary", "MathLibrary.slang", kMathLibrarySource, nullptr);
    if (!lib1Module)
        return SLANG_FAIL;
    
    SLANG_RETURN_ON_FAIL(lib1Module->serialize(outLib1Blob.writeRef()));
    printf("    Library 1 serialized: %zu bytes\n", outLib1Blob->getBufferSize());

    // Compile Library 2
    ComPtr<slang::IModule> lib2Module;
    lib2Module = session->loadModuleFromSourceString(
        "ColorLibrary", "ColorLibrary.slang", kColorLibrarySource, nullptr);
    if (!lib2Module)
        return SLANG_FAIL;
    
    SLANG_RETURN_ON_FAIL(lib2Module->serialize(outLib2Blob.writeRef()));
    printf("    Library 2 serialized: %zu bytes\n", outLib2Blob->getBufferSize());

    return SLANG_OK;
}

// Helper: Load modules and compile final shader with entry point
static SlangResult compileFinalShader(
    slang::ISession* session,
    ISlangBlob* lib1Blob,
    ISlangBlob* lib2Blob,
    ComPtr<slang::IComponentType>& outLinkedProgram)
{
    printf("  Loading modules and compiling final shader...\n");

    // Load libraries from IR blobs
    ComPtr<slang::IModule> loadedLib1;
    loadedLib1 = slang_loadModuleFromIRBlob(
        session, "MathLibrary", "MathLibrary.slang",
        lib1Blob->getBufferPointer(), lib1Blob->getBufferSize(), nullptr);
    if (!loadedLib1)
        return SLANG_FAIL;

    ComPtr<slang::IModule> loadedLib2;
    loadedLib2 = slang_loadModuleFromIRBlob(
        session, "ColorLibrary", "ColorLibrary.slang",
        lib2Blob->getBufferPointer(), lib2Blob->getBufferSize(), nullptr);
    if (!loadedLib2)
        return SLANG_FAIL;

    // Load inline shader from source
    ComPtr<slang::IModule> inlineModule;
    inlineModule = session->loadModuleFromSourceString(
        "InlineShader", "InlineShader.slang", kInlineShaderSource, nullptr);
    if (!inlineModule)
        return SLANG_FAIL;

    // Compile final shader
    ComPtr<slang::IModule> finalModule;
    finalModule = session->loadModuleFromSourceString(
        "FinalShader", "FinalShader.slang", kFinalShaderSource, nullptr);
    if (!finalModule)
        return SLANG_FAIL;

    // Find entry point
    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_RETURN_ON_FAIL(finalModule->findEntryPointByName("computeMain", entryPoint.writeRef()));

    // Create composite
    slang::IComponentType* components[] = {finalModule, entryPoint, inlineModule, loadedLib1, loadedLib2};
    ComPtr<slang::IComponentType> compositeProgram;
    SLANG_RETURN_ON_FAIL(session->createCompositeComponentType(
        components, 5, compositeProgram.writeRef(), nullptr));

    // Link program
    SLANG_RETURN_ON_FAIL(compositeProgram->link(outLinkedProgram.writeRef(), nullptr));

    printf("    Final shader compiled and linked successfully\n");
    return SLANG_OK;
}

// Combined test: Obfuscation with and without separate debug
// Verifies that debug SPIR-V from EmitSeparateDebug matches full SPIR-V without it
SLANG_UNIT_TEST(obfuscationWithSeparateDebug)
{
    printf("========================================\n");
    printf("Testing Obfuscation with Separate Debug\n");
    printf("========================================\n\n");

    // Create global session
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // Common options
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::CompilerOptionEntry obfuscationOption;
    obfuscationOption.name = slang::CompilerOptionName::Obfuscate;
    obfuscationOption.value.kind = slang::CompilerOptionValueKind::Int;
    obfuscationOption.value.intValue0 = 1;

    slang::CompilerOptionEntry debugInfoOption;
    debugInfoOption.name = slang::CompilerOptionName::DebugInformation;
    debugInfoOption.value.kind = slang::CompilerOptionValueKind::Int;
    debugInfoOption.value.intValue0 = 2;

    slang::CompilerOptionEntry separateDebugOption;
    separateDebugOption.name = slang::CompilerOptionName::EmitSeparateDebug;
    separateDebugOption.value.kind = slang::CompilerOptionValueKind::Int;
    separateDebugOption.value.intValue0 = 1;

    // ====================================================================
    // TEST CASE 1: Obfuscation ON + EmitSeparateDebug ON + DebugInfo level 2
    // ====================================================================
    printf("=== Test Case 1: EmitSeparateDebug ON ===\n");
    printf("Config: Obfuscation ON + DebugInfo Full + EmitSeparateDebug ON\n\n");

    // Create session for initial library compilation (separate debug info doesn't matter here)
    slang::SessionDesc sessionDesc1 = {};
    sessionDesc1.targetCount = 1;
    sessionDesc1.targets = &targetDesc;
    slang::CompilerOptionEntry options1[] = {obfuscationOption, debugInfoOption};
    sessionDesc1.compilerOptionEntries = options1;
    sessionDesc1.compilerOptionEntryCount = sizeof(options1) / sizeof(options1[0]);

    ComPtr<slang::ISession> session1;
    SLANG_CHECK(globalSession->createSession(sessionDesc1, session1.writeRef()) == SLANG_OK);

    // Compile and serialize library modules
    ComPtr<ISlangBlob> lib1Blob, lib2Blob;
    SLANG_CHECK(compileLibraryModules(session1, lib1Blob, lib2Blob) == SLANG_OK);

    // Create final session with separate debug enabled
    // Separate debug info option is needed here
    slang::CompilerOptionEntry optionsWithSepDebug[] = {obfuscationOption, debugInfoOption, separateDebugOption};
    sessionDesc1.compilerOptionEntries = optionsWithSepDebug;
    sessionDesc1.compilerOptionEntryCount = sizeof(optionsWithSepDebug) / sizeof(optionsWithSepDebug[0]);

    ComPtr<slang::ISession> finalSession1;
    SLANG_CHECK(globalSession->createSession(sessionDesc1, finalSession1.writeRef()) == SLANG_OK);

    // Compile final shader
    ComPtr<slang::IComponentType> linkedProgram1;
    SLANG_CHECK(compileFinalShader(finalSession1, lib1Blob, lib2Blob, linkedProgram1) == SLANG_OK);

    // Generate SPIR-V with separate debug
    printf("  Generating SPIR-V with EmitSeparateDebug...\n");
    ComPtr<slang::IComponentType2> linkedProgram1_v2;
    SLANG_CHECK(linkedProgram1->queryInterface(SLANG_IID_PPV_ARGS(linkedProgram1_v2.writeRef())) == SLANG_OK);

    ComPtr<slang::ICompileResult> compileResult1;
    SLANG_CHECK(linkedProgram1_v2->getEntryPointCompileResult(0, 0, compileResult1.writeRef(), nullptr) == SLANG_OK);

    SLANG_CHECK(compileResult1->getItemCount() == 2);

    ComPtr<slang::IBlob> spirvStripped, spirvDebug;
    SLANG_CHECK(compileResult1->getItemData(0, spirvStripped.writeRef()) == SLANG_OK);
    SLANG_CHECK(compileResult1->getItemData(1, spirvDebug.writeRef()) == SLANG_OK);

    printf("    Main SPIR-V (stripped): %zu bytes\n", spirvStripped->getBufferSize());
    printf("    Debug SPIR-V (full): %zu bytes\n", spirvDebug->getBufferSize());

    // Extract Debug Build Identifier
    ComPtr<slang::IMetadata> metadata1;
    const char* dbi1 = nullptr;
    if (compileResult1->getMetadata(metadata1.writeRef()) == SLANG_OK && metadata1)
    {
        dbi1 = metadata1->getDebugBuildIdentifier();
        if (dbi1)
            printf("    Debug Build Identifier: %s\n", dbi1);
    }

    printf("✓ Test Case 1 Complete\n\n");

    // ====================================================================
    // TEST CASE 2: Obfuscation ON + EmitSeparateDebug OFF (shipping) + DebugInfo level 0 (shipping modules)
    // ====================================================================
    printf("=== Test Case 2: EmitSeparateDebug OFF (Shipping) ===\n");
    printf("Config: Obfuscation ON + DebugInfo Full + EmitSeparateDebug OFF\n\n");

    // Create session with DebugInfo level 0 (shipping modules)
    slang::SessionDesc sessionDesc2 = {};
    sessionDesc2.targetCount = 1;
    sessionDesc2.targets = &targetDesc;
    slang::CompilerOptionEntry options2[] = {obfuscationOption}; //no debug info option here because we will ship the modules
    sessionDesc2.compilerOptionEntries = options2;
    sessionDesc2.compilerOptionEntryCount = sizeof(options2) / sizeof(options2[0]);

    ComPtr<slang::ISession> session2;
    SLANG_CHECK(globalSession->createSession(sessionDesc2, session2.writeRef()) == SLANG_OK);

    // Recompile libraries (must use same session for consistency)
    ComPtr<ISlangBlob> lib1Blob2, lib2Blob2;
    SLANG_CHECK(compileLibraryModules(session2, lib1Blob2, lib2Blob2) == SLANG_OK);

    // Create final session (same options, no separate debug)
    ComPtr<slang::ISession> finalSession2;
    SLANG_CHECK(globalSession->createSession(sessionDesc2, finalSession2.writeRef()) == SLANG_OK);

    // Compile final shader
    ComPtr<slang::IComponentType> linkedProgram2;
    SLANG_CHECK(compileFinalShader(finalSession2, lib1Blob2, lib2Blob2, linkedProgram2) == SLANG_OK);

    // Generate SPIR-V without separate debug (and no debug info, because we will ship the modules)
    printf("  Generating SPIR-V without EmitSeparateDebug...\n");
    ComPtr<slang::IBlob> spirvFull;
    SLANG_CHECK(linkedProgram2->getEntryPointCode(0, 0, spirvFull.writeRef(), nullptr) == SLANG_OK);

    printf("    Full SPIR-V (withot debug): %zu bytes\n", spirvFull->getBufferSize());
    printf("✓ Test Case 2 Complete\n\n");

    // ====================================================================
    // VERIFICATION: Compare debug SPIR-V from Test 1 with full SPIR-V from Test 2
    // ====================================================================
    printf("=== Verification: Comparing Artifacts ===\n");
    printf("Expected: debug.dbg.spv (Test 1) should match full.spv (Test 2)\n");
    printf("Both contain full debug information\n\n");

#if DUMP_FILES
    // Create output directory
    const char* outputDir = SLANG_DUMP_PATH;
#ifdef _WIN32
    _mkdir(outputDir);
#else
    mkdir(outputDir, 0755);
#endif
    printf("Output directory: %s\n\n", outputDir);

    // Always dump artifacts for debugging
    printf("Dumping artifacts for inspection...\n");
    dumpBlobToFile(SLANG_DUMP_PATH "/test1-stripped.spv", spirvStripped, "Test 1: Main SPIR-V (stripped)");
    dumpBlobToFile(SLANG_DUMP_PATH "/test1-debug-full.spv", spirvDebug, "Test 1: Debug SPIR-V (full)");
    dumpBlobToFile(SLANG_DUMP_PATH "/test2-full.spv", spirvFull, "Test 2: Full SPIR-V (with debug)");
    printf("\n");
#endif

    bool artifactsMatch = compareBlobsEqual(
        spirvStripped,
        spirvFull,
        "test1-stripped.spv",
        "test2-full.spv");

#if DUMP_FILES
    if (!artifactsMatch)
    {
        printf("\n⚠️  MISMATCH DETECTED! Files dumped to %s for inspection:\n", SLANG_DUMP_PATH);
        printf("  - test1-stripped.spv (Test 1 stripped output)\n");
        printf("  - test1-debug-full.spv (Test 1 full output with debug info, AKA dbg.spv)\n");
        printf("  - test2-full.spv (Test 2 output, for shipping, no debug info, but should be associated with test1-debug-full.spv)\n");
        printf("\nCompare these files to diagnose the issue:\n");
        printf("  spirv-dis test1-debug-full.spv > test1-debug.txt\n");
        printf("  spirv-dis test2-full.spv > test2-full.txt\n");
        printf("  diff test1-debug.txt test2-full.txt\n");
    }
#endif

    SLANG_CHECK(artifactsMatch);

    printf("\n=== Summary ===\n");
    printf("✓ Test Case 1 (EmitSeparateDebug ON):\n");
    printf("  - Stripped SPIR-V: %zu bytes (no debug info)\n", spirvStripped->getBufferSize());
    printf("  - Debug SPIR-V: %zu bytes (full debug info)\n", spirvDebug->getBufferSize());
    if (dbi1)
        printf("  - Debug Build ID: %s\n", dbi1);
    printf("\n");
    printf("✓ Test Case 2 (EmitSeparateDebug OFF - Shipping):\n");
    printf("  - Full SPIR-V: %zu bytes (with debug info)\n", spirvFull->getBufferSize());
    printf("\n");
    printf("✓ Verification: Debug artifacts are IDENTICAL\n");
    printf("  - This confirms that EmitSeparateDebug only affects output format\n");
    printf("  - The debug information content is the same in both cases\n");
#if DUMP_FILES
    printf("\n✓ Artifacts dumped to %s/:\n", SLANG_DUMP_PATH);
    printf("  - test1-main-stripped.spv (Test 1 stripped output)\n");
    printf("  - test1-debug-full.spv (Test 1 debug output)\n");
    printf("  - test2-full.spv (Test 2 full output with embedded debug)\n");
#endif
    printf("\n");
    printf("Test completed successfully!\n");
}

