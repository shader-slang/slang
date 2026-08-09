// Regression test: arithmetic on a value loaded from a `unorm`/`snorm` texture must use
// the floating-point opcode. The loaded element has type `Attributed(Float, unorm)`, and
// the SPIR-V emitter classified that as non-floating-point, selecting `OpIAdd` with a
// `%float` result type -- invalid SPIR-V, rejected by spirv-val with "Expected int scalar
// or vector type as Result Type: IAdd".

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute

RWTexture2D<unorm float> u1;
RWTexture2D<snorm float> s1;
RWStructuredBuffer<float> outputBuffer;

//SPV-NOT: OpIAdd %float

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint2 c = tid.xy;
    //SPV: OpFAdd %float
    outputBuffer[0] = u1[c] + s1[c];
}
