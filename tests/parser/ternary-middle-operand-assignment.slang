// The middle operand of `?:` is a full expression, as in C, C++, HLSL and GLSL, so an assignment or
// a comma operator there needs no parentheses (shader-slang/slang#13870).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -Wno-41024
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -Wno-41024
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -std -xslang 2026

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(2, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    bool c = tid.x == 1;
    int base = int(tid.x) * 8;

    float b = 6.0;
    float g = c ? 1.0 : 10.0;
    b > g ? b /= 2.0 : 0.0;
    outputBuffer[base + 0] = b;

    // A bit-flag update with an assignment in the middle operand. The last operand keeps its
    // parentheses because it is still parsed at the `?:` level.
    uint flags = 5;
    uint m = 3;
    flags = c ? flags |= m : (flags &= ~m);
    outputBuffer[base + 1] = float(flags);

    // A comma operator in the middle operand: the increment runs, then `a * 2` is the value.
    int a = 3;
    int x = c ? a += 1, a * 2 : 0;
    outputBuffer[base + 2] = float(x);
    outputBuffer[base + 3] = float(a);

    // Inside an argument list the comma in the middle operand belongs to the `?:`, and the
    // comma after the last operand still separates the arguments.
    outputBuffer[base + 4] = max(c ? a, 7.0 : 2.0, 0.5);

    // A plain assignment, a nested `?:` whose middle operand is an assignment, and a parenthesized
    // `?:`, which Slang 2026 parses as a single element rather than as a tuple.
    float s = 1.0;
    float r = c ? s = 5.0 : 2.0;
    outputBuffer[base + 5] = r + s;
    float t = 0.0;
    float n = c ? c ? t = 4.0 : 2.0 : 3.0;
    outputBuffer[base + 6] = n + t;
    outputBuffer[base + 7] = (c ? t += 1.0, t * 10.0 : 9.0);
}

// Thread 0 (false branch)
// CHECK: 6.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 9.0
// Thread 1 (true branch)
// CHECK-NEXT: 3.0
// CHECK-NEXT: 7.0
// CHECK-NEXT: 8.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 7.0
// CHECK-NEXT: 10.0
// CHECK-NEXT: 8.0
// CHECK-NEXT: 50.0
