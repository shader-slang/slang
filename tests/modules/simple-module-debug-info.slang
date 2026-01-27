//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -emit-spirv-directly

// Test that SPIRV debug information is correctly generated for functions
// defined via __include vs directly in the module file.
//
// This is a minimal reproduction case for the bug where functions from
// __include files are missing DebugFunctionDefinition.

import simple_module;

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // TestFunctionA is defined in simple-module-impl.slang via __include
    int a = TestFunctionA(3);
    // TestFunctionB is defined directly in simple-module.slang
    int b = TestFunctionB(4);
    outputBuffer[0] = a;
    outputBuffer[1] = b;
}

// Verify DebugFunctionDefinition for TestFunctionB (defined directly in module file)
// CHECK-DAG: [[TESTFUNCB_STR:%[0-9]+]] = OpString "TestFunctionB"
// CHECK-DAG: [[TESTFUNCB_FUNC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[TESTFUNCB_STR]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[TESTFUNCB_FUNC]] %TestFunctionB

// Verify DebugFunctionDefinition for TestFunctionA (defined via __include)
// CHECK-DAG: [[TESTFUNCA_STR:%[0-9]+]] = OpString "TestFunctionA"
// CHECK-DAG: [[TESTFUNCA_FUNC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[TESTFUNCA_STR]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[TESTFUNCA_FUNC]] %TestFunctionA
