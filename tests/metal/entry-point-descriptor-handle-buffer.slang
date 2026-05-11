// Regression for #11066: Handle-wrapped buffer params were missing
// [[buffer(n)]], so Metal's implicit declaration-order slots diverged from
// Slang reflection and writes landed in the wrong buffer.  The compute
// test below catches the corruption: if bindings are misaligned, the
// write of 0.125f into dataBuffer aliases outputBuffer[0] and the BUFFER
// check fails.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -mtl -shaderobj -output-using-type

//TEST_INPUT: set dataBuffer = ubuffer(stride=4, count=4096)
uniform RWStructuredBuffer<float>.Handle dataBuffer;

//TEST_INPUT: set biasBuffer = ubuffer(stride=4, count=64)
uniform RWStructuredBuffer<float>.Handle biasBuffer;

//TEST_INPUT:ubuffer(stride=4, count=5):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

void fillBuffer(uint tid, float value)
{
    dataBuffer[64] = value;
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_GroupIndex)
{
    outputBuffer[0] = 1;
    outputBuffer[1] = 1;
    outputBuffer[2] = 1;
    outputBuffer[3] = 1;
    outputBuffer[4] = 1;

    if (tid == 0 && biasBuffer[0] == 12345.0f)
        outputBuffer[0] = 9;

    fillBuffer(tid, 0.125f);
}

// Emit order follows the entry-point KernelContext struct.
// CHECK: outputBuffer{{[_0-9]*}} {{\[\[}}buffer(
// CHECK: biasBuffer{{[_0-9]*}} {{\[\[}}buffer(
// CHECK: dataBuffer{{[_0-9]*}} {{\[\[}}buffer(

// Metal's compiler must accept the generated MSL end-to-end.
// METALLIB: @computeMain

// With the bug, writes through `dataBuffer` alias outputBuffer and the
// final outputBuffer contents differ from the five 1s written above.
// BUFFER: type: uint32_t
// BUFFER-NEXT: {{^}}1{{$}}
// BUFFER-NEXT: {{^}}1{{$}}
// BUFFER-NEXT: {{^}}1{{$}}
// BUFFER-NEXT: {{^}}1{{$}}
// BUFFER-NEXT: {{^}}1{{$}}
