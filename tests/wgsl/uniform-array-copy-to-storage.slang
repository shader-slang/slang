//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry computeMain -stage compute

// std140 stores each `float` array element as a 16-byte vector, but std430 does not, so a
// constant-buffer array copied into a structured buffer is read one element at a time, and
// each element read is a typed `.x` extraction from its vector.

struct S
{
    float a[3];
};

ConstantBuffer<S> gC;
RWStructuredBuffer<S> gOut;

// CHECK: fn computeMain()
// CHECK-NOT: {{^ *}}gC_0.a_0.data_0[i32({{[0-9]}})].x;
// CHECK: var [[A1:_S[0-9]+]] : f32 = gC_0.a_0.data_0[i32(1)].x;
// CHECK: gOut_0[i32(0)].a_0[i32(0)] = gC_0.a_0.data_0[i32(0)].x;
// CHECK: gOut_0[i32(0)].a_0[i32(1)] = [[A1]];
// CHECK: var [[B2:_S[0-9]+]] : f32 = gC_0.a_0.data_0[i32(2)].x;
// CHECK: gOut_0[i32(1)].a_0[i32(2)] = [[B2]];

[numthreads(1, 1, 1)]
void computeMain()
{
    gOut[0] = gC;
    gOut[1].a = gC.a;
}
