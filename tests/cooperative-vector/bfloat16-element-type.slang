// shader-slang/slang#12411: `BFloat16` must be usable as a `CoopVec` element type.
// Compile-only: cooperative-vector kernels cannot be executed in the test environment, so this
// checks front-end acceptance and SPIR-V codegen rather than runtime results.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;
ByteAddressBuffer input;
StructuredBuffer<float> sbuf;
StructuredBuffer<BFloat16> bsbuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<BFloat16, 4> v = coopVecLoad<4, BFloat16>(input);
    outputBuffer[0] = float(v[0]);

    // Control: an arithmetic element type keeps its operator set.
    CoopVec<half, 4> h = coopVecLoad<4, half>(input);
    h = h + h;
    outputBuffer[1] = float(h[0]);

    // Spelling every generic argument (including the buffer layout) must stay unambiguous.
    CoopVec<float, 4> s = coopVecLoad<4, float, DefaultDataLayout>(sbuf);
    outputBuffer[2] = float(s[0]);

    // Inferred element type from a structured buffer, including the non-arithmetic BFloat16 element.
    CoopVec<BFloat16, 4> b = coopVecLoad<4>(bsbuf);
    outputBuffer[3] = float(b[0]);
}

//CHECK: OpCooperativeVectorLoadNV
