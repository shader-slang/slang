// unit-test-original-issue-reproduction.cpp

#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test reproduction of the exact original issue
SLANG_UNIT_TEST(originalIssueReproduction)
{
    // Exact code from the original issue
    const char* testSource = R"(
public interface IBase
{
    public void step(inout float f);
}

public struct Impl : IBase
{
    public void step(inout float f)
    {
        f += 1.0f;
    }
}
)";

    auto session = spCreateSession();
    auto request = spCreateCompileRequest(session);
    spAddCodeGenTarget(request, SLANG_DXBC);
    int tuIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "tu1");
    spAddTranslationUnitSourceString(request, tuIndex, "internalFile", testSource);
    spCompile(request);

    auto testBody = [&]()
    {
        auto reflection = slang::ShaderReflection::get(request);
        
        // Load the module, get the layout, get findTypeByName("Impl")
        auto implType = reflection->findTypeByName("Impl");
        SLANG_CHECK_ABORT(implType != nullptr);
        
        // then findFunctionByNameInType on step
        auto stepFunction = reflection->findFunctionByNameInType(implType, "step");
        SLANG_CHECK_ABORT(stepFunction != nullptr);
        
        // I get a slang::FunctionReflection that returns NULL for getName()
        auto functionName = stepFunction->getName();
        printf("Function name from original issue scenario: %s\n", functionName ? functionName : "(null)");
        
        // The issue was that this returned null - it should return "step"
        SLANG_CHECK_ABORT(functionName != nullptr);
        SLANG_CHECK_ABORT(strcmp(functionName, "step") == 0);
        
        printf("Original issue fixed: getName() now returns '%s'\n", functionName);
    };

    testBody();

    spDestroyCompileRequest(request);
    spDestroySession(session);
}