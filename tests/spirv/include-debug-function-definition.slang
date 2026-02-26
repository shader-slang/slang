//TEST:SIMPLE(filecheck=CHECK):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain
//
// Regression test for Issue #9806:
// Functions defined in a file brought in via `__include` must get SPIR-V debug info
// (in particular DebugFunctionDefinition).
//
// This test is designed to exercise the `findAndIncludeFile()` path so that the
// included file is added to TranslationUnitRequest::getSourceFiles().

module include_debug_function_definition;

__include "include-debug-function-definition-impl";

RWStructuredBuffer<int> outputBuffer;

int TestFunctionB(int x)
{
    return x + 200;
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // TestFunctionA is defined in the __include'd file.
    int a = TestFunctionA(3);
    // TestFunctionB is defined in this file.
    int b = TestFunctionB(4);
    outputBuffer[0] = a;
    outputBuffer[1] = b;
}

// Verify DebugFunctionDefinition for TestFunctionB (defined in this file)
// CHECK-DAG: [[TESTFUNCB_STR:%[0-9]+]] = OpString "TestFunctionB"
// CHECK-DAG: [[TESTFUNCB_FUNC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[TESTFUNCB_STR]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[TESTFUNCB_FUNC]] %TestFunctionB

// Verify DebugFunctionDefinition for TestFunctionA (defined via __include)
// CHECK-DAG: [[TESTFUNCA_STR:%[0-9]+]] = OpString "TestFunctionA"
// CHECK-DAG: [[TESTFUNCA_FUNC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[TESTFUNCA_STR]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[TESTFUNCA_FUNC]] %TestFunctionA

