// An aligned byte-address access to an array is emitted as one typed access when the promised
// alignment is a multiple of the array's size, which comes from the element stride. `T` is
// 24 bytes without rounding and 32 bytes with `-layout-rules-version 202c`, so `T[2]` is 48 or
// 64 bytes. A 64-byte promise therefore takes the typed path only with 202c. Without it, the
// access is split into per-field loads and stores, and `t[1].b[2]` is read at byte 24 + 20 = 44.

//TEST:SIMPLE(filecheck=OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout
//TEST:SIMPLE(filecheck=NEW):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c

struct S
{
    double x;
    int y;
};

struct T
{
    S a;
    float b[3];
};

ByteAddressBuffer bab;
RWByteAddressBuffer rwbab;
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    T t[2] = bab.LoadAligned<T[2]>(tid.x * 64, 64);
    output[0] = asuint(t[1].b[2]);
    rwbab.Store<T[2]>(tid.x * 128, t, 64);
}

// OLD-NOT: _Array_natural_T2
// OLD: OpIAdd %uint %{{[0-9]+}} %uint_44
// OLD-NOT: _Array_natural_T2

// NEW: OpDecorate %_arr_T_scalar_rounded_int_2 ArrayStride 32
// NEW: OpAccessChain %_ptr_StorageBuffer__Array_scalar_rounded_T2 %bab %int_0
// NEW: OpAccessChain %_ptr_StorageBuffer__Array_scalar_rounded_T2 %rwbab{{(_[0-9]+)?}} %int_0
