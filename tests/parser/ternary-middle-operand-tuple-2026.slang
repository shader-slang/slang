// In Slang 2026 a top-level comma inside parentheses separates tuple elements, while a comma inside
// the middle operand of `?:` stays a comma operator (shader-slang/slang#13870).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -std -xslang 2026
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -std -xslang 2026

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(2, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    bool c = tid.x == 1;
    float t = 5.0;
    let tup = (c ? t += 1.0, 1.0 : 2.0, 3.0);
    outputBuffer[tid.x * 3 + 0] = tup._0;
    outputBuffer[tid.x * 3 + 1] = tup._1;
    outputBuffer[tid.x * 3 + 2] = t;
}

// Thread 0 (false branch): tup = (2.0, 3.0), t unchanged
// CHECK: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 5.0
// Thread 1 (true branch): tup = (1.0, 3.0), t incremented
// CHECK-NEXT: 1.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 6.0
