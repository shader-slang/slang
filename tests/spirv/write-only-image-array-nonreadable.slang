//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK):-target spirv -stage compute -entry computeMain -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=NEG):-target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=NEG):-target spirv -stage compute -entry computeMain -emit-spirv-via-glsl

// An array of write-only textures must get the `NonReadable` decoration on
// its variable, the same as a single write-only texture does, and the direct
// SPIR-V path must agree with the path through GLSL. The GLSL path names
// variables with a `_0` suffix.
// See: https://github.com/shader-slang/slang/issues/13895

// CHECK-DAG: OpDecorate %single{{(_0)?}} NonReadable
// CHECK-DAG: OpDecorate %sized{{(_0)?}} NonReadable
// CHECK-DAG: OpDecorate %unsized{{(_0)?}} NonReadable
// CHECK-DAG: OpDecorate %layered{{(_0)?}} NonReadable
// CHECK-DAG: OpDecorate %entryPointParams_epArr{{(_0)?}} NonReadable
// CHECK-DAG: OpDecorate %pb_arr{{(_0)?}} NonReadable
// CHECK-DAG: OpDecorate %readWrite{{(_0)?}} Binding
// CHECK-DAG: OpDecorate %readOnly{{(_0)?}} Binding

// Read-write and read-only texture arrays must stay readable. The read-write
// array declares its format so that it does not itself need
// StorageImageReadWithoutFormat.
// NEG-NOT: OpCapability StorageImageReadWithoutFormat
// NEG-NOT: OpDecorate %readWrite{{(_0)?}} NonReadable
// NEG-NOT: OpDecorate %readOnly{{(_0)?}} NonReadable

struct Params
{
    WTexture2D<float4> arr[2];
}

WTexture2D<float4> single;
WTexture2D<float4> sized[4];
WTexture2D<float4> unsized[];
WTexture2DArray<float4> layered[2];
ParameterBlock<Params> pb;

[format("rgba32f")]
RWTexture2D<float4> readWrite[2];
Texture2D<float4> readOnly[2];

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID, uniform WTexture2D<float4> epArr[2])
{
    float4 value = readOnly[id.x].Load(int3(0, 0, 0));
    readWrite[id.x].Store(int2(0, 0), value);
    single.Store(int2(0, 0), value);
    sized[id.x].Store(int2(0, 0), value);
    unsized[id.x].Store(int2(0, 0), value);
    layered[id.x].Store(int3(0, 0, 0), value);
    epArr[id.x].Store(int2(0, 0), value);
    pb.arr[id.x].Store(int2(0, 0), value);
}
