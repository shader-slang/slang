// unit-test-translation-unit-import.cpp

#include "../../slang.h"

#include <stdio.h>
#include <stdlib.h>

#include "tools/unit-test/slang-unit-test.h"
#include "../../slang-com-ptr.h"

using namespace Slang;

// Test that the API supports discovering previously checked translation unit in the same
// FrontEndCompileRequest.
SLANG_UNIT_TEST(translationUnitImport)
{
    // Source for the first translation unit.
    const char* generatedSource =
        "int f() {"
        "   return 5;"
        "};";

    // Source for the second translation unit that imports the first translation unit.
    // The import should succeed and `f` should be visible to this module.
    const char* userSource =
        R"(
        import generatedUnit;

        [shader("compute")]
        [numthreads(4,1,1)]
        void computeMain(
            uint3 sv_dispatchThreadID : SV_DispatchThreadID,
            uniform RWStructuredBuffer<int> buffer)
        {
            buffer[sv_dispatchThreadID.x] = f();
        }
        )";    
    auto session = spCreateSession();
    auto request = spCreateCompileRequest(session);
    spAddCodeGenTarget(request, SLANG_HLSL);
    int generatedTranslationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "generatedUnit");
    spAddTranslationUnitSourceString(
        request, generatedTranslationUnitIndex, "generatedFile", generatedSource);
    int entryPointTranslationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "userUnit");
    spAddTranslationUnitSourceString(
        request, entryPointTranslationUnitIndex, "userFile", userSource);
    spAddEntryPoint(request, entryPointTranslationUnitIndex, "computeMain", SLANG_STAGE_COMPUTE);

    auto compileResult = spCompile(request);
    SLANG_CHECK(compileResult == SLANG_OK);

    Slang::ComPtr<ISlangBlob> outBlob;
    spGetEntryPointCodeBlob(request, 0, 0, outBlob.writeRef());
    SLANG_CHECK(outBlob && outBlob->getBufferSize() != 0);

    spDestroyCompileRequest(request);
    spDestroySession(session);
}

