// Copying a struct out of a constant buffer and into a structured buffer must preserve its values
// when the two buffers store the struct differently (shader-slang/slang#13379).
//
// One matrix of `S` is row-major and the other column-major, so every target stores at least one
// of them differently from its logical form. Both matrices are 32 bytes under std140 as well as
// the natural layout, so the same constant-buffer data works for every test below.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -llvm -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type -profile spirv_1_3

struct S
{
    row_major float2x4 a;
    column_major float4x2 b;
    float4 v;
};

//TEST_INPUT: set C = cbuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0])
ConstantBuffer<S> C;

//TEST_INPUT: set copies = ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=80)
RWStructuredBuffer<S> copies;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

void write(int base, S r)
{
    for (int row = 0; row < 2; row++)
        for (int col = 0; col < 4; col++)
            outputBuffer[base + row * 4 + col] = r.a[row][col];
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 2; col++)
            outputBuffer[base + 8 + row * 2 + col] = r.b[row][col];
    for (int i = 0; i < 4; i++)
        outputBuffer[base + 16 + i] = r.v[i];
}

// Each copy reads back as `a`, then `b` row by row, then `v`.
// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 5.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 9.000000
// CHECK-NEXT: 13.000000
// CHECK-NEXT: 10.000000
// CHECK-NEXT: 14.000000
// CHECK-NEXT: 11.000000
// CHECK-NEXT: 15.000000
// CHECK-NEXT: 12.000000
// CHECK-NEXT: 16.000000
// CHECK-NEXT: 17.000000
// CHECK-NEXT: 18.000000
// CHECK-NEXT: 19.000000
// CHECK-NEXT: 20.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 5.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 9.000000
// CHECK-NEXT: 13.000000
// CHECK-NEXT: 10.000000
// CHECK-NEXT: 14.000000
// CHECK-NEXT: 11.000000
// CHECK-NEXT: 15.000000
// CHECK-NEXT: 12.000000
// CHECK-NEXT: 16.000000
// CHECK-NEXT: 17.000000
// CHECK-NEXT: 18.000000
// CHECK-NEXT: 19.000000
// CHECK-NEXT: 20.000000
[numthreads(1, 1, 1)]
void computeMain()
{
    S t = C;
    copies[0] = t;
    copies[1] = C;

    write(0, copies[0]);
    write(20, copies[1]);
}
