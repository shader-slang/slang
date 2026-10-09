//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -fvk-use-scalar-layout
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Copying a whole value, a nested struct or an array out of a constant buffer
// that names `ScalarDataLayout` into a structured buffer. Both buffers store
// packed vectors, but in distinct storage types (one per address space), so
// the copy unpacks each member and packs it again rather than copying one
// storage type into the other. A direct copy between two storage types is
// only possible on SPIR-V.

struct Inner
{
    float a;
    float3 v;
};

struct Shapes
{
    float x;
    Inner inner;
    float3 arr[2];
    float3x3 m;
    float2x3 n;
};

ConstantBuffer<Shapes, ScalarDataLayout> shapes;
RWStructuredBuffer<Shapes> outShapes;
RWStructuredBuffer<Inner> outInner;

[numthreads(1, 1, 1)]
void computeMain()
{
    outShapes[0] = shapes;
    outInner[0] = shapes.inner;
    outShapes[1].arr = shapes.arr;
    outShapes[2].m = shapes.m;
    outShapes[3].n = shapes.n;
}

// METAL-LABEL: computeMain
// METAL: (&_S{{[0-9]+}}->inner_0)->v_0 = packed_float3(
// METAL: (&_S{{[0-9]+}}->arr_0)->data_0[int(1)] = packed_float3(
// METAL: array<packed_float3, int(3)> {{.*}} packed_float3(
// METAL: array<packed_float2, int(3)> {{.*}} packed_float2(
// METAL: _S{{[0-9]+}}->v_0 = packed_float3(
// METAL: ->arr_0)->data_0[int(1)] = packed_float3(
// METAL: ->m_0)->data_1 =
// METAL: ->n_0)->data_2 =

// METALLIB: define void @computeMain
