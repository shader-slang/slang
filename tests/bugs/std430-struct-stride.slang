//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -emit-spirv-directly -profile spirv_1_5 -entry main -stage compute

// Test that std430 struct stride is rounded up to alignment.
// A struct ending at byte 28 with alignment 8 (due to uint64_t) must have stride 32.

struct Inner
{
    uint     a;    // @  0
    uint     b;    // @  4
    uint64_t ptr;  // @  8  (forces alignment = 8)
    int      c;    // @ 16
    int      d;    // @ 20
    int      e;    // @ 24
};

struct Parent
{
    Inner inner;
    uint  after;
};

struct PushConstants { uint64_t params; };

// CHECK-DAG: OpMemberDecorate %Parent_std430 1 Offset 32

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform PushConstants pc)
{
    Parent k = *reinterpret<LayoutPtr<Parent, Std430DataLayout>>(pc.params);
    reinterpret<LayoutPtr<Parent, Std430DataLayout>>(pc.params).after = uint(k.inner.e) + 1u;
}
