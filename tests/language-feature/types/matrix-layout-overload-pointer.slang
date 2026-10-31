// Pointer overloads that differ only in the pointee's matrix layout are distinct functions
// (shader-slang/slang#13383). This runs only under render-test's row-major default: under a
// column-major default the CPU target fails to compile `&plain` for a plain `Ptr<float2x3>`
// parameter, with or without overloads (shader-slang/slang#13405).

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float pickPointer(Ptr<row_major float2x3> p) { return 1.0; }
float pickPointer(Ptr<float2x3> p) { return 2.0; }

[numthreads(1, 1, 1)]
void computeMain()
{
    var row : row_major float2x3 = {};
    var plain : float2x3 = {};
    outputBuffer[0] = pickPointer(&row);
    outputBuffer[1] = pickPointer(&plain);
}

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
