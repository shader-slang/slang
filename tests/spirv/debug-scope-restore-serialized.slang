// The early-inlined leaf leaves an ordinary scope restore inside importedOuter.
// Serialize that one-operand IRDebugScope, then inline the imported function and
// verify that both its entry and restored scope retain the same inline chain.
//TEST:COMPILE: tests/spirv/debug-scope-restore-module.slang -g3 -o tests/spirv/debug-scope-restore-module.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/spirv/debug-scope-restore-module.slang-module -target spirv -entry main -stage compute -g3 -O0

import "debug-scope-restore-module";
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int result = importedOuter(int(tid.x));
    outputBuffer[tid.x] = result;
}

// CHECK-DAG: %[[NAME:[0-9]+]] = OpString "importedOuter"
// CHECK-DAG: %[[OUTER:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[NAME]]
// CHECK-DAG: %[[MAINNAME:[0-9]+]] = OpString "main"
// CHECK-DAG: %[[MAIN:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[MAINNAME]]
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[OUTER]] %[[SITE:[0-9]+]]{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[OUTER]] %[[SITE]]{{$}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugScope %[[MAIN]]{{$}}
// CHECK-NEXT: OpExtInst %void %{{[0-9]+}} DebugValue %result
// CHECK-NOT: OpExtInst %void %{{[0-9]+}} DebugNoScope
