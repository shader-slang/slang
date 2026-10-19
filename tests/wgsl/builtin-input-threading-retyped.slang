//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry main -stage compute

// Entry-point inputs declared with a type that WGSL does not allow for the built-in
// (`int3 : SV_DispatchThreadID`, `int : SV_WaveLaneCount`) are passed to callees as parameters
// of the declared type. The entry point's built-in parameter has the WGSL type (`vec3<u32>`,
// `u32`), so it is converted at the start of the entry point, before the call of the
// module-scope initializer that reads the input (see builtin-input-threading.slang).

// CHECK-NOT: var<private> {{.*}} : vec3<i32>;
// CHECK-NOT: var<private> {{.*}} : i32;{{$}}
// CHECK: fn signedIndex_{{[0-9]+}}( [[TID_IN:gTidSigned_[0-9]+]] : vec3<i32>) -> i32
// CHECK-NEXT: {
// CHECK-NEXT: return [[TID_IN]].x;
// CHECK: fn [[INIT:[A-Za-z0-9_]+]]( {{gTidSigned_[0-9]+}} : vec3<i32>) -> i32
// CHECK: var<private> gInit_{{[0-9]+}} : i32;
// CHECK: fn laneCountSigned_{{[0-9]+}}( [[LANES_IN:gLanesSigned_[0-9]+]] : i32) -> i32
// CHECK-NEXT: {
// CHECK-NEXT: return [[LANES_IN]];
// CHECK: fn leaf_{{[0-9]+}}( v_{{[0-9]+}} : i32,  {{gLanesSigned_[0-9]+}} : i32) -> i32
// CHECK: fn main(@builtin(global_invocation_id) [[TID:[A-Za-z0-9_]+]] : vec3<u32>, @builtin(subgroup_size) [[LANES:[A-Za-z0-9_]+]] : u32)
// CHECK-NEXT: {
// CHECK-DAG: var [[LANES_I:gLanesSigned_[0-9]+]] : i32 = i32([[LANES]]);
// CHECK-DAG: var [[TID_I:gTidSigned_[0-9]+]] : vec3<i32> = vec3<i32>([[TID]]);
// CHECK: gInit_{{[0-9]+}} = [[INIT]]([[TID_I]]);
// CHECK: signedIndex_{{[0-9]+}}([[TID_I]])
// CHECK: leaf_{{[0-9]+}}({{.*}}, [[LANES_I]])

// TINT: OpGroupNonUniformIAdd

in int3 gTidSigned : SV_DispatchThreadID;
in int gLanesSigned : SV_WaveLaneCount;

RWStructuredBuffer<int> outputBuffer;

int signedIndex() { return gTidSigned.x; }

int laneCountSigned() { return gLanesSigned; }

static int gInit = gTidSigned.y;

int initRead() { return gInit; }

int leaf(int v)
{
    int r = v;
    for (int i = 0; i < 64 / laneCountSigned(); i++)
        r = WaveActiveSum(r);
    return r;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void main()
{
    outputBuffer[signedIndex()] = leaf(initRead());
}
