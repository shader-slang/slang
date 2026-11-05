// Regression test for #14184: Negating a scalar uint in WGSL must emit `(u32(0) - x)`
// because WGSL has no unary `-` operator for unsigned types.
// Signed integer and floating-point negations must continue emitting native `-`.

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry computeMain -stage compute
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu -compute -shaderobj -output-using-type

// WGSL-DAG: (u32(0) - {{.*}})
// WGSL-DAG: (vec2<u32>(0) - {{.*}})
// WGSL-NOT: (i32(0) -
// WGSL-NOT: (f32(0) -

// TINT: OpEntryPoint

// CHECK: type: uint32_t
// CHECK-NEXT: 4294967291
// CHECK-NEXT: 4294967286
// CHECK-NEXT: type: int32_t
// CHECK-NEXT: -5
// CHECK-NEXT: type: float
// CHECK-NEXT: -7.000000

//TEST_INPUT:ubuffer(data=[5 10], stride=4):name=inputU
StructuredBuffer<uint> inputU;

//TEST_INPUT:ubuffer(data=[5], stride=4):name=inputI
StructuredBuffer<int> inputI;

//TEST_INPUT:ubuffer(data=[7.0], stride=4):name=inputF
StructuredBuffer<float> inputF;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBufferI
RWStructuredBuffer<int> outputBufferI;

//TEST_INPUT:ubuffer(data=[0.0], stride=4):out,name=outputBufferF
RWStructuredBuffer<float> outputBufferF;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint u = inputU[0];
    outputBuffer[0] = -u;

    uint2 u2 = uint2(inputU[0], inputU[1]);
    uint2 negU2 = -u2;
    outputBuffer[1] = negU2.y;

    int i = inputI[0];
    outputBufferI[0] = -i;

    float f = inputF[0];
    outputBufferF[0] = -f;
}
