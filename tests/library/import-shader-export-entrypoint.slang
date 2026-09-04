// An imported `[shader]`+`export` parametered function that this compilation does
// not select as an entry point must be treated as an ordinary exported function,
// not an unlaid-out entry point (shader-slang/slang#12564).

//TEST:COMPILE: tests/library/import-shader-export-entrypoint-module.slang -o tests/library/import-shader-export-entrypoint-module.slang-module
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import import_shader_export_entrypoint_module;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = int(tid.x) * 2;
    // CHECK: 0
    // CHECK-NEXT: 2
    // CHECK-NEXT: 4
    // CHECK-NEXT: 6
}
