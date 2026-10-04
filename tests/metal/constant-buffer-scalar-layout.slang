//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -fvk-use-scalar-layout
//TEST:REFLECTION(filecheck=REFL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// A Metal constant buffer that names `ScalarDataLayout` uses natural layout,
// like a device buffer: its vectors and matrices are stored as MSL packed
// vectors, and reflection reports the matching offsets. A constant buffer with
// the default layout keeps the native MSL layout, also when the same struct is
// used by a scalar-layout constant buffer in the same module, and so do
// entry-point uniforms.

struct Args
{
    float3 A;
    float3 B;
};

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
    float2 f2;
    float tail;
};

StructuredBuffer<Args> sb;
ConstantBuffer<Args, ScalarDataLayout> cb;
ConstantBuffer<Args> cbDefault;
ConstantBuffer<Shapes, ScalarDataLayout> shapes;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain(uniform Args epArgs)
{
    output[0] = sb[0].A.x + cb.A.x + cb.B.y + cbDefault.B.z + epArgs.B.x;
    output[1] = shapes.x + shapes.inner.v.y + shapes.arr[1].z + shapes.m[2][1] + shapes.f2.y +
                shapes.tail;
}

// `sb`, `cb` and `cbDefault` share `Args` but get three distinct storage
// types: packed copies for the device buffer and the scalar constant buffer,
// and the unlowered `Args_0`, with native `float3` fields, for the default one.
// METAL: struct Args_natural_{{[0-9]+}}
// METAL-NEXT: {
// METAL-NEXT: packed_float3 A_0;
// METAL-NEXT: packed_float3 B_0;
// METAL: struct Args_natural_{{[0-9]+}}
// METAL-NEXT: {
// METAL-NEXT: packed_float3 A_0;
// METAL-NEXT: packed_float3 B_0;
// METAL: struct Args_0
// METAL-NEXT: {
// METAL-NEXT: {{^ +}}float3 A_0;
// METAL-NEXT: {{^ +}}float3 B_0;
// METAL: struct Inner_natural_0
// METAL-NEXT: {
// METAL-NEXT: float a_0;
// METAL-NEXT: packed_float3 v_0;
// METAL-DAG: array<packed_float3, int(2)> data_
// METAL-DAG: array<packed_float3, int(3)> data_
// METAL-DAG: packed_float2 f2_0;
// METAL-DAG: Args_natural_{{[0-9]+}} device* sb_
// METAL-DAG: Args_natural_{{[0-9]+}} constant* cb_
// METAL-DAG: Args_0 constant* cbDefault_
// METAL-DAG: Shapes_natural_{{[0-9]+}} constant* shapes_

// Entry-point uniforms are a constant buffer with the default layout, so they
// stay native even under `-fvk-use-scalar-layout`.
// METAL-DAG: {{^ +}}Args_0 epArgs_0;
// METAL-DAG: EntryPointParams_0 constant* entryPointParams_

// REFL: "name": "sb",
// REFL: "name": "A",
// REFL: "binding": {"kind": "uniform", "offset": 0, "size": 12, "elementStride": 4}
// REFL: "name": "B",
// REFL: "binding": {"kind": "uniform", "offset": 12, "size": 12, "elementStride": 4}

// REFL: "name": "cb",
// REFL: "name": "A",
// REFL: "binding": {"kind": "uniform", "offset": 0, "size": 12, "elementStride": 4}
// REFL: "name": "B",
// REFL: "binding": {"kind": "uniform", "offset": 12, "size": 12, "elementStride": 4}

// REFL: "name": "cbDefault",
// REFL: "name": "A",
// REFL: "binding": {"kind": "uniform", "offset": 0, "size": 16, "elementStride": 4}
// REFL: "name": "B",
// REFL: "binding": {"kind": "uniform", "offset": 16, "size": 16, "elementStride": 4}

// REFL: "name": "shapes",
// REFL: "name": "x",
// REFL: "binding": {"kind": "uniform", "offset": 0, "size": 4, "elementStride": 0}
// REFL: "name": "inner",
// REFL: "binding": {"kind": "uniform", "offset": 4, "size": 16, "elementStride": 0}
// REFL: "name": "arr",
// REFL: "binding": {"kind": "uniform", "offset": 20, "size": 24, "elementStride": 12}
// REFL: "name": "m",
// REFL: "binding": {"kind": "uniform", "offset": 44, "size": 36, "elementStride": 0}
// REFL: "name": "f2",
// REFL: "binding": {"kind": "uniform", "offset": 80, "size": 8, "elementStride": 4}
// REFL: "name": "tail",
// REFL: "binding": {"kind": "uniform", "offset": 88, "size": 4, "elementStride": 0}

// METALLIB: define void @computeMain
