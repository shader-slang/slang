// A struct copy between two buffers whose element types are lowered under different layouts must
// produce valid SPIR-V, with or without `OpCopyLogical` (shader-slang/slang#13379).
//
// The `cbuffer` element of `S` keeps its matrix as `OpTypeMatrix`, but the pointee of `S*` wraps it
// in a `_MatrixStorage` struct, so the two storage types do not logically match. SPIR-V before
// 1.4 has no `OpCopyLogical`, so every logical copy has to be expanded there.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV13): -target spirv -entry main -stage compute -profile spirv_1_3
//TEST:SIMPLE(filecheck=IR): -target spirv -entry main -stage compute -dump-ir-after lowerBufferElementTypeToStorageType
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute -DSAME_BUFFER
//TEST:SIMPLE(filecheck=SPV13): -target spirv -entry main -stage compute -profile spirv_1_3 -DSAME_BUFFER
//TEST:SIMPLE(filecheck=TWIN_IR): -target spirv -entry main -stage compute -dump-ir-after lowerBufferElementTypeToStorageType -DSAME_BUFFER

struct S
{
    float2x2 m;
    bool c;
    float a[3];
};

#if defined(SAME_BUFFER)
RWStructuredBuffer<S> b;
#else
cbuffer C
{
    S s;
    S* p;
}
#endif

// CHECK: OpEntryPoint
// SPV13: OpEntryPoint
// SPV13-NOT: OpCopyLogical

// The `cbuffer` value is loaded into a local of its layout-free twin with a logical copy, and then
// unpacked and packed into the pointee of `p`, with no logical copy for the store.
// IR-LABEL: func %main
// IR: copyLogical(%{{.*}}, %{{.*}})
// IR-NOT: copyLogical
// IR: call %packStorage
// IR-NOT: copyLogical
// IR: return_val

// Storing the twin back to a buffer whose element has the same config is a second logical copy,
// with no unpacking or packing.
// TWIN_IR-LABEL: func %main
// TWIN_IR: copyLogical(%{{.*}}, %{{.*}})
// TWIN_IR-NOT: packStorage
// TWIN_IR: copyLogical(%{{.*}}, %{{.*}})
// TWIN_IR-NOT: packStorage
// TWIN_IR: return_val
[numthreads(1, 1, 1)]
void main()
{
#if defined(SAME_BUFFER)
    S t = b[0];
    b[1] = t;
#else
    S t = s;
    *p = t;
#endif
}
