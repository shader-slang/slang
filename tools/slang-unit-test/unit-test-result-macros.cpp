#include "slang-com-helper.h"
#include "unit-test/slang-unit-test.h"

SLANG_UNIT_TEST(resultMacrosAsStatements)
{
    for (bool condition : {false, true})
    {
        for (SlangResult result : {SLANG_OK, SLANG_E_INVALID_ARG})
        {
            int evaluationCount = 0;
            bool tookElse = false;
            bool continued = false;
            auto checkState = [&]()
            {
                SLANG_CHECK(evaluationCount == (condition ? 1 : 0));
                SLANG_CHECK(tookElse == !condition);
                SLANG_CHECK(continued == (!condition || SLANG_SUCCEEDED(result)));
                evaluationCount = 0;
                tookElse = false;
                continued = false;
            };

            // Keep these branches unbraced: each macro and its semicolon must form one statement.
            auto returnResult = [&]() -> SlangResult
            {
                if (condition)
                    SLANG_RETURN_ON_FAIL((++evaluationCount, result));
                else
                    tookElse = true;
                continued = true;
                return SLANG_OK;
            };
            SLANG_CHECK(returnResult() == (condition ? result : SLANG_OK));
            checkState();

            auto returnVoid = [&]()
            {
                if (condition)
                    SLANG_RETURN_VOID_ON_FAIL((++evaluationCount, result));
                else
                    tookElse = true;
                continued = true;
            };
            returnVoid();
            checkState();

            auto returnBool = [&]()
            {
                if (condition)
                    SLANG_RETURN_FALSE_ON_FAIL((++evaluationCount, result));
                else
                    tookElse = true;
                continued = true;
                return true;
            };
            SLANG_CHECK(returnBool() == (!condition || SLANG_SUCCEEDED(result)));
            checkState();

            auto returnPointer = [&]() -> void*
            {
                if (condition)
                    SLANG_RETURN_NULL_ON_FAIL((++evaluationCount, result));
                else
                    tookElse = true;
                continued = true;
                return &evaluationCount;
            };
            SLANG_CHECK(
                returnPointer() ==
                ((!condition || SLANG_SUCCEEDED(result)) ? &evaluationCount : nullptr));
            checkState();
        }
    }
}
