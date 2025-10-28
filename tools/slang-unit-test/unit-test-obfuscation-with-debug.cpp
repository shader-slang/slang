// unit-test-obfuscation-with-debug.cpp

// Define this to dump all intermediate files and SPIR-V output to disk
#define DUMP_FILES 0

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>

#if DUMP_FILES
#define SLANG_DUMP_PATH "slang_dump_spirv_asm_obfuscation_ON"
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
    slang::IComponentType* components[] = { finalModule, entryPoint, inlineModule, loadedLib1, loadedLib2 };
    ComPtr<slang::IComponentType> compositeProgram;
    SLANG_RETURN_ON_FAIL(session->createCompositeComponentType(
        components, 5, compositeProgram.writeRef(), nullptr));

    // Link program
    SLANG_RETURN_ON_FAIL(compositeProgram->link(outLinkedProgram.writeRef(), nullptr));

    printf("    Final shader compiled and linked successfully\n");
    return SLANG_OK;
}

// Helper: Check if SPIR-V assembly contains debug instructions
static bool spirvAsmContainsDebugInstructions(const char* spirvAsm)
{
    // Check for common debug instructions in SPIR-V assembly
    const char* debugInstructions[] = {
        "OpSource ",
        "OpSourceContinued ",
        "OpSourceExtension ",
        "OpName ",
        "OpMemberName ",
        "OpLine ",
        "OpModuleProcessed ",
        "DebugSource",
        "DebugCompilationUnit",
        "DebugTypeBasic",
        "DebugTypeFunction",
        "DebugFunction",
        "DebugLexicalBlock",
        "DebugLocalVariable",
        "DebugExpression"
    };

    for (const char* debugInst : debugInstructions)
    {
        if (strstr(spirvAsm, debugInst) != nullptr)
        {
            return true;
        }
    }

    return false;
}

// Helper: Find DBI in SPIR-V assembly
static bool findDebugBuildIdentifierInSpirv(const char* spirvAsm, const char* expectedDBI)
{
    // Search for the DBI string in the SPIR-V assembly
    return strstr(spirvAsm, expectedDBI) != nullptr;
}

// Helper: Count OpName instructions to check obfuscation
static int countOpNameInstructions(const char* spirvAsm)
{
    int count = 0;
    const char* searchStr = "OpName ";
    const char* pos = spirvAsm;

    while ((pos = strstr(pos, searchStr)) != nullptr)
    {
        count++;
        pos += strlen(searchStr);
    }

    return count;
}

// Helper: Verify separate debug SPIR-V outputs
static SlangResult verifySeparateDebugOutput(
    slang::IBlob* strippedSpirv,
    slang::IBlob* debugSpirv,
    const char* debugBuildIdentifier)
{
    if (!strippedSpirv || !debugSpirv)
    {
        printf("ERROR: One or both SPIR-V blobs are NULL\n");
        return SLANG_FAIL;
    }

    if (!debugBuildIdentifier)
    {
        printf("ERROR: Debug Build Identifier is NULL\n");
        return SLANG_FAIL;
    }

    printf("  Stripped SPIR-V size: %zu bytes\n", strippedSpirv->getBufferSize());
    printf("  Debug SPIR-V size: %zu bytes\n", debugSpirv->getBufferSize());
    printf("  Debug Build Identifier: %s\n", debugBuildIdentifier);

    // Get SPIR-V assembly as strings
    const char* strippedAsm = static_cast<const char*>(strippedSpirv->getBufferPointer());
    const char* debugAsm = static_cast<const char*>(debugSpirv->getBufferPointer());

    // Verify that debug SPIR-V is larger (contains more debug info)
    if (debugSpirv->getBufferSize() <= strippedSpirv->getBufferSize())
    {
        printf("WARNING: Debug SPIR-V (%zu bytes) is not larger than stripped SPIR-V (%zu bytes)\n",
            debugSpirv->getBufferSize(), strippedSpirv->getBufferSize());
        printf("This might indicate that debug info was not properly separated.\n");
    }

    printf("\n  --- Verification 1: No accidental debug info in stripped SPIR-V ---\n");
    bool strippedHasDebugInstructions = spirvAsmContainsDebugInstructions(strippedAsm);
    if (strippedHasDebugInstructions)
    {
        printf("  ✗ FAIL: Stripped SPIR-V contains debug instructions (OpSource, OpName, DebugExpression, etc.)\n");
        printf("  This indicates debug info was not properly stripped!\n");
        return SLANG_FAIL;
    }
    else
    {
        printf("  ✓ PASS: Stripped SPIR-V has no debug instructions\n");
    }

    printf("\n  --- Verification 2: Obfuscation check ---\n");
    int strippedOpNameCount = countOpNameInstructions(strippedAsm);
    int debugOpNameCount = countOpNameInstructions(debugAsm);
    printf("  OpName count in stripped SPIR-V: %d\n", strippedOpNameCount);
    printf("  OpName count in debug SPIR-V: %d\n", debugOpNameCount);

    if (strippedOpNameCount > 0)
    {
        printf("  ✗ WARNING: Stripped SPIR-V has %d OpName instructions\n", strippedOpNameCount);
        printf("  Obfuscation may not have removed all names.\n");
    }
    else
    {
        printf("  ✓ PASS: Stripped SPIR-V has no OpName instructions (fully obfuscated)\n");
    }

    if (debugOpNameCount > 0)
    {
        printf("  ✓ Debug SPIR-V has %d OpName instructions (preserved for debugging)\n", debugOpNameCount);
    }

    printf("\n  --- Verification 3: DBI matches in both files ---\n");
    bool strippedHasDBI = findDebugBuildIdentifierInSpirv(strippedAsm, debugBuildIdentifier);
    bool debugHasDBI = findDebugBuildIdentifierInSpirv(debugAsm, debugBuildIdentifier);

    printf("  DBI found in stripped SPIR-V: %s\n", strippedHasDBI ? "YES" : "NO");
    printf("  DBI found in debug SPIR-V: %s\n", debugHasDBI ? "YES" : "NO");

    if (!strippedHasDBI)
    {
        printf("  ✗ FAIL: DBI not found in stripped SPIR-V\n");
        return SLANG_FAIL;
    }

    if (!debugHasDBI)
    {
        printf("  ✗ FAIL: DBI not found in debug SPIR-V\n");
        return SLANG_FAIL;
    }

    printf("  ✓ PASS: DBI '%s' found in both files\n", debugBuildIdentifier);
    printf("\n  ✓ All verifications passed!\n");

    return SLANG_OK;
}


// Obfuscation with separate debug workflow test
// Execute the workflow:
// 1. Compile slang_modules with -g2, obfuscation ON
// 2. Compile the final shader with -g2, obfuscation ON, separate debug ON
// 3. Verify:
//    a) Obfuscation correctly happened (no OpName instructions in stripped.spv)
//    b) stripped.spv has no accidental debug info (no OpSource, OpLine, etc.)
//    c) DBI in stripped.spv matches DBI in dbg.spv
//    
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
    // Use SPIR-V assembly format for easier verification (human-readable text)
    targetDesc.format = SLANG_SPIRV_ASM;
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
    // Step 1: Compile slang modules with -g2 + Obfuscation ON
    // ====================================================================
    printf("=== Step 1: Compile *slang_modules with -g2 + Obfuscation ON ===\n");

    // Create session for initial library compilation (separate debug info doesn't matter here)
    slang::SessionDesc sessionDesc1 = {};
    sessionDesc1.targetCount = 1;
    sessionDesc1.targets = &targetDesc;
    slang::CompilerOptionEntry module_options[] = { debugInfoOption, obfuscationOption };
    //    slang::CompilerOptionEntry module_options[] = {debugInfoOption};

    sessionDesc1.compilerOptionEntries = module_options;
    sessionDesc1.compilerOptionEntryCount = sizeof(module_options) / sizeof(module_options[0]);

    ComPtr<slang::ISession> session1;
    SLANG_CHECK(globalSession->createSession(sessionDesc1, session1.writeRef()) == SLANG_OK);

    // Compile and serialize library modules
    ComPtr<ISlangBlob> lib1Blob, lib2Blob;
    SLANG_CHECK(compileLibraryModules(session1, lib1Blob, lib2Blob) == SLANG_OK);

    // ====================================================================
    // Step 2: Generate SPIR-V with separate debug
    // ====================================================================
    printf("=== Step 2: Compile final spirv with -g2 + Obfuscation ON + separate debug ON ===\n");

    // Create final session with separate debug enabled
    slang::CompilerOptionEntry optionsWithSepDebug[] = { debugInfoOption, obfuscationOption, separateDebugOption };
    // slang::CompilerOptionEntry optionsWithSepDebug[] = {debugInfoOption, separateDebugOption};

    sessionDesc1.compilerOptionEntries = optionsWithSepDebug;
    sessionDesc1.compilerOptionEntryCount = sizeof(optionsWithSepDebug) / sizeof(optionsWithSepDebug[0]);

    ComPtr<slang::ISession> finalSession1;
    SLANG_CHECK(globalSession->createSession(sessionDesc1, finalSession1.writeRef()) == SLANG_OK);

    // Compile final shader
    ComPtr<slang::IComponentType> linkedProgram1;
    SLANG_CHECK(compileFinalShader(finalSession1, lib1Blob, lib2Blob, linkedProgram1) == SLANG_OK);

    // Query for IComponentType2 interface
    ComPtr<slang::IComponentType2> linkedProgram1_v2;
    SlangResult queryResult = linkedProgram1->queryInterface(SLANG_IID_PPV_ARGS(linkedProgram1_v2.writeRef()));
    SLANG_CHECK(queryResult == SLANG_OK);
    SLANG_CHECK(linkedProgram1_v2 != nullptr);
    printf("  IComponentType2 interface obtained successfully\n");

    // Get compile result with separate debug
    ComPtr<slang::ICompileResult> compileResult;
    ComPtr<ISlangBlob> compileResultDiagnostics;
    SlangResult compileStatus = linkedProgram1_v2->getEntryPointCompileResult(
        0, // Entry point index
        0, // Target index
        compileResult.writeRef(),
        compileResultDiagnostics.writeRef());

    if (compileResultDiagnostics && compileResultDiagnostics->getBufferSize() > 0)
    {
        printf("  Compile result diagnostics:\n%.*s\n",
            (int)compileResultDiagnostics->getBufferSize(),
            (const char*)compileResultDiagnostics->getBufferPointer());
    }

    SLANG_CHECK(compileStatus == SLANG_OK);

    if (!compileResult)
    {
        printf("ERROR: getEntryPointCompileResult returned SLANG_OK but compileResult is NULL!\n");
        printf("This likely means EmitSeparateDebug is not properly enabled or supported for this target.\n");
    }
    SLANG_CHECK(compileResult != nullptr);

    printf("  ICompileResult obtained successfully\n");

    // Verify we got exactly 2 items (stripped + debug)
    int itemCount = compileResult->getItemCount();
    printf("  Item count: %d\n", itemCount);
    SLANG_CHECK(itemCount == 2);

    // Extract the two SPIR-V outputs
    ComPtr<slang::IBlob> spirvStripped, spirvDebug;
    SLANG_CHECK(compileResult->getItemData(0, spirvStripped.writeRef()) == SLANG_OK);
    SLANG_CHECK(compileResult->getItemData(1, spirvDebug.writeRef()) == SLANG_OK);
    SLANG_CHECK(spirvStripped != nullptr);
    SLANG_CHECK(spirvDebug != nullptr);

    // Extract metadata and Debug Build Identifier
    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK(compileResult->getMetadata(metadata.writeRef()) == SLANG_OK);

    const char* debugBuildIdentifier = metadata->getDebugBuildIdentifier();
    SLANG_CHECK(debugBuildIdentifier != nullptr);

    printf("  Main SPIR-V (stripped): %zu bytes\n", spirvStripped->getBufferSize());
    printf("  Debug SPIR-V (full): %zu bytes\n", spirvDebug->getBufferSize());
    printf("  Debug Build Identifier: %s\n", debugBuildIdentifier);
    printf("  ✓ Step 2 Complete\n\n");

    // ====================================================================
    // Step 3: Verification
    // ====================================================================
    printf("=== Step 3: Verification ===\n");
    SLANG_CHECK(verifySeparateDebugOutput(spirvStripped, spirvDebug, debugBuildIdentifier) == SLANG_OK);

#if DUMP_FILES
    // Create output directory
    const char* outputDir = SLANG_DUMP_PATH;
#ifdef _WIN32
    _mkdir(outputDir);
#else
    mkdir(outputDir, 0755);
#endif
    printf("  Output directory: %s\n", outputDir);

    // Dump artifacts for inspection
    printf("  Dumping artifacts for inspection...\n");
    dumpBlobToFile(SLANG_DUMP_PATH "/stripped-asm_obfuscation_ON.spvasm", spirvStripped, "  Main SPIR-V ASM (stripped)");
    dumpBlobToFile(SLANG_DUMP_PATH "/debug-full-asm_obfuscation_ON.spvasm", spirvDebug, "  Debug SPIR-V ASM (full)");
#endif // DUMP_FILES

    printf("\nTest completed successfully!\n");
}
