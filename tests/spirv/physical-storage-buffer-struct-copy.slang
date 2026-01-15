// Test that copying structs from/to PhysicalStorageBuffer pointers
// includes the required Aligned memory operand in SPIR-V.
// This was a bug where OpStore didn't have Aligned, causing SPIR-V validation errors.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry CS_Main

struct Foo {
    uint x, y, z, w;
};

// Check that both load and store have Aligned operand for PhysicalStorageBuffer access.
// CHECK: OpLoad %Foo_natural %{{[0-9]+}} Aligned
// CHECK: OpStore %{{[0-9]+}} %{{[0-9]+}} Aligned

[numthreads(64)]
void CS_Main(uniform Foo* src, uniform Foo* dst, uint tid: SV_DispatchThreadID) {
    dst[tid] = src[tid];
}
