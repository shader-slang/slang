// unit-test-find-function-by-name-in-type-overloaded.cpp

#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test for the specific issue reported: when findFunctionByNameInType finds
// an overloaded function, getName() should return the name, not nullptr
SLANG_UNIT_TEST(findFunctionByNameInTypeOverloaded)
{
    // This is the code from the original issue, modified to create an overload
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
    
    // Add an overload to make it truly overloaded
    public void step(inout int i)
    {
        i += 1;
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
        
        // Find the Impl type
        auto implType = reflection->findTypeByName("Impl");
        SLANG_CHECK_ABORT(implType != nullptr);
        
        // Find the step function within the Impl type
        auto stepFunction = reflection->findFunctionByNameInType(implType, "step");
        SLANG_CHECK_ABORT(stepFunction != nullptr);
        
        // Check if it's overloaded
        bool isOverloaded = stepFunction->isOverloaded();
        printf("Function is overloaded: %s\n", isOverloaded ? "yes" : "no");
        
        if (isOverloaded) {
            printf("Number of overloads: %d\n", stepFunction->getOverloadCount());
        }
        
        // This is the bug: getName() returns nullptr for overloaded functions
        auto functionName = stepFunction->getName();
        printf("Function name: %s\n", functionName ? functionName : "(null)");
        
        // This should pass and return the name for overloaded functions
        SLANG_CHECK_ABORT(functionName != nullptr);
        SLANG_CHECK_ABORT(strcmp(functionName, "step") == 0);
        
        // Test accessing individual overloads
        if (isOverloaded) {
            for (int i = 0; i < stepFunction->getOverloadCount(); i++) {
                auto overload = stepFunction->getOverload(i);
                SLANG_CHECK_ABORT(overload != nullptr);
                auto overloadName = overload->getName();
                printf("Overload %d name: %s\n", i, overloadName ? overloadName : "(null)");
                // Each individual overload should also have a name
                SLANG_CHECK_ABORT(overloadName != nullptr);
                SLANG_CHECK_ABORT(strcmp(overloadName, "step") == 0);
            }
        }
    };

    testBody();

    spDestroyCompileRequest(request);
    spDestroySession(session);
}