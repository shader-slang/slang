// A struct copy between two buffers whose element types are lowered under different layouts must
// produce valid SPIR-V, with or without `OpCopyLogical` (shader-slang/slang#13379).
//
// The `cbuffer` element of `S` keeps its matrix as `OpTypeMatrix`, but the pointee of `S*` wraps it
// in a `_MatrixStorage` struct, so the two storage types do not logically match. SPIR-V before
// 1.4 has no `OpCopyLogical`, so the copy into the local variable has to be expanded there.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV13): -target spirv -entry main -stage compute -profile spirv_1_3
//TEST:SIMPLE(filecheck=IR): -target spirv -entry main -stage compute -dump-ir-after lowerBufferElementTypeToStorageType

struct S
{
    float2x2 m;
    bool c;
};

cbuffer C
{
    S s;
    S* p;
}

// CHECK: OpEntryPoint
// SPV13: OpEntryPoint
// SPV13-NOT: OpCopyLogical

// The only logical copy left is between the `cbuffer` storage type and its layout-free twin.
// IR-LABEL: func %main
// IR: copyLogical(%{{.*}}, %{{.*}})
// IR-NOT: copyLogical
// IR: call %packStorage
// IR: return_val
[numthreads(1, 1, 1)]
void main()
{
    S t = s;
    *p = t;
}
