// `rwbab.Store<U[2]>` writes the elements of `U[2]` one field at a time, at offsets from the element
// stride. `U` is 16 bytes and `U.w` is at byte 12 without rounding; with
// `-layout-rules-version 202c` `U` is 24 bytes and `U.w` is at byte 16, so the second element's
// `z.y` and `w` move from bytes 24 and 28 to bytes 32 and 40.

//TEST:SIMPLE(filecheck=OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout
//TEST:SIMPLE(filecheck=NEW):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c

struct S
{
    double x;
    int y;
};

struct U
{
    S z;
    float w;
};

RWByteAddressBuffer rwbab;
RWStructuredBuffer<U> sb;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    U u[2] = { sb[0], sb[1] };
    rwbab.Store<U[2]>(tid.x * 64, u);
}

// OLD: OpIAdd %uint %{{[0-9]+}} %uint_12
// OLD: OpIAdd %uint %{{[0-9]+}} %uint_24
// OLD: OpIAdd %uint %{{[0-9]+}} %uint_28
// OLD-NOT: %uint_40

// NEW-NOT: OpIAdd %uint %{{[0-9]+}} %uint_12
// NEW: OpIAdd %uint %{{[0-9]+}} %uint_16
// NEW: OpIAdd %uint %{{[0-9]+}} %uint_32
// NEW: OpIAdd %uint %{{[0-9]+}} %uint_40
