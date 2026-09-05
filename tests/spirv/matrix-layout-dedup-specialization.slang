//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -matrix-layout-row-major

// With -matrix-layout-row-major, `row_major float4x4` and `float4x4` are the same type, so `Foo`
// must be specialized only once. That requires matrix layouts to be resolved before
// specialization; otherwise the two spellings key as different generic arguments.

struct Foo<T>
{
    T v;
}

StructuredBuffer<Foo<row_major float4x4> > a;
StructuredBuffer<Foo<float4x4> > b;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = a[0].v[0][0] + b[0].v[0][0];
}

// A second `Foo` would be emitted under the same name, suffixed _0 by the disassembler.
// The positive check anchors the negative one so a rename cannot make it vacuous.
// CHECK: %Foo_std430 = OpTypeStruct
// CHECK-NOT: %Foo_std430_0 = OpTypeStruct
