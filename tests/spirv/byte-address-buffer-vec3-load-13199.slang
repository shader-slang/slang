//TEST:SIMPLE(filecheck=MAT43):-target spirv-asm -stage compute -entry csMat43 -matrix-layout-row-major
//TEST:SIMPLE(filecheck=VEC3):-target spirv-asm -stage compute -entry csVec3
//TEST:SIMPLE(filecheck=SPVSCALAR):-target spirv-asm -stage compute -entry csVec3 -fvk-use-scalar-layout
//TEST:SIMPLE(filecheck=MAT33):-target spirv-asm -stage compute -entry csMat33 -matrix-layout-row-major
//TEST:SIMPLE(filecheck=ARR):-target spirv-asm -stage compute -entry csArr -matrix-layout-row-major
//TEST:SIMPLE(filecheck=DOUBLE3):-target spirv-asm -stage compute -entry csDouble3
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry csVec3
//TEST:SIMPLE(filecheck=GLSLDX):-target glsl -stage compute -entry csVec3 -fvk-use-dx-layout
//TEST:SIMPLE(filecheck=GLSLC):-target glsl -stage compute -entry csVec3 -fvk-use-c-layout
//TEST:SIMPLE(filecheck=GLSLSCALAR):-target glsl -stage compute -entry csVec3 -fvk-use-scalar-layout

// shader-slang/slang#13199.
//
// On the targets that legalize a byte-address buffer into a typed structured buffer (SPIR-V and
// GLSL), when the reinterpret buffer is std430-laid-out a `float3` leaf must be accessed through a
// tight scalar `float` runtime array (ArrayStride 4), never a padded `float3` runtime array. A
// `float3` reinterpreted as a std430 structured-buffer element gets stride 16, but the element index
// is `byteOffset / naturalStride(float3)` = `byteOffset / 12`; the two disagree, so element `k`
// would sit at byte `k*16` while the tight field walk and store intend `k*12`, corrupting the data
// at any natural-stride-aligned nonzero offset. The std430-backed entries — SPIR-V under the default
// layout and GLSL under the default, `-fvk-use-dx-layout`, and `-fvk-use-c-layout` layouts — assert
// the tight scalar array is used and no padded 3-component array is created; `DOUBLE3` covers the
// structurally distinct 8-byte element (`double3` 24→32, so a tight `double` array at ArrayStride
// 8). Under `-fvk-use-scalar-layout` the reinterpret buffer is itself tight, so a wide `float3` /
// `vec3` access is correct and must be left intact; the `SPVSCALAR` (direct SPIR-V) and `GLSLSCALAR`
// (textual GLSL) controls guard both branches against over-scalarization. A value-level `-vk`
// round-trip lives in `tests/compute/byte-address-buffer-vec3-13199.slang`.

RWByteAddressBuffer buffer;

// The reported shape: a struct whose row-major `float4x3` sits at natural field offset 24 (a
// multiple of the 12-byte natural `float3` stride), so each row is a `float3` at a
// natural-stride-aligned offset that requires scalarization.
struct Mat43Struct
{
    uint a;
    float b;
    float4 c;
    float4x3 d;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void csMat43()
{
    buffer.Store(48, buffer.Load<Mat43Struct>(0));
}
// MAT43: OpEntryPoint
// MAT43: OpDecorate %_runtimearr_float ArrayStride 4
// MAT43-NOT: OpTypeRuntimeArray %v3float
// MAT43-NOT: OpLoad %v3float

// A bare `float3` at byte 12 (a nonzero multiple of the natural stride) requires scalarization; the
// load reads the three tight source floats at byte offsets 12,16,20 (indices 3,4,5) and the store
// writes the three tight destination floats at byte offsets 48,52,56 (indices 12,13,14).
[shader("compute")]
[numthreads(1, 1, 1)]
void csVec3()
{
    buffer.Store(48, buffer.Load<float3>(12));
}
// VEC3: OpEntryPoint
// VEC3: OpDecorate %_runtimearr_float ArrayStride 4
// VEC3-NOT: OpTypeRuntimeArray %v3float
// VEC3: OpAccessChain %_ptr_StorageBuffer_float {{.*}} %uint_3
// VEC3-NOT: OpLoad %v3float
// VEC3: OpLoad %float
// VEC3-NOT: OpLoad %v3float
// VEC3: OpAccessChain %_ptr_StorageBuffer_float {{.*}} %uint_12
// VEC3-NOT: OpLoad %v3float
// VEC3-NOT: OpTypeRuntimeArray %v3float
// Control: under `-fvk-use-scalar-layout` the direct-SPIR-V reinterpret buffer is itself tight
// (ArrayStride 12), so the wide `v3float` access is correct and must be left intact — this guards
// the SPIR-V branch against over-scalarization, the counterpart of the `GLSLSCALAR` leg. The
// positive `OpLoad %v3float` is the proof the load stayed wide (scalarization would remove it).
// SPVSCALAR: OpEntryPoint
// SPVSCALAR: OpDecorate %_runtimearr_v3float ArrayStride 12
// SPVSCALAR: OpTypeRuntimeArray %v3float
// SPVSCALAR: OpLoad %v3float
// SPVSCALAR-NOT: OpLoad %float
// GLSL: #version
// GLSL-NOT: vec3 _data
// GLSL: float _data
// GLSL-NOT: vec3 _data
// GLSLDX: #version
// GLSLDX-NOT: vec3 _data
// GLSLDX: float _data
// GLSLDX-NOT: vec3 _data
// GLSLC: #version
// GLSLC-NOT: vec3 _data
// GLSLC: float _data
// GLSLC-NOT: vec3 _data
// Under `layout(scalar)` a wide `vec3` element is already tight, so it is correctly left wide.
// GLSLSCALAR: #version
// GLSLSCALAR-NOT: float _data
// GLSLSCALAR: vec3 _data
// GLSLSCALAR-NOT: float _data

// A bare `float3x3` has `float3` rows at byte offsets 0, 12, 24; the latter two are
// natural-stride-aligned and require scalarization.
[shader("compute")]
[numthreads(1, 1, 1)]
void csMat33()
{
    buffer.Store(48, buffer.Load<float3x3>(0));
}
// MAT33: OpEntryPoint
// MAT33: OpDecorate %_runtimearr_float ArrayStride 4
// MAT33-NOT: OpTypeRuntimeArray %v3float
// MAT33-NOT: OpLoad %v3float

// A `float3[2]` array reaches the padded structured-buffer path directly (not through a vector or
// matrix case), so it requires the same scalarization.
[shader("compute")]
[numthreads(1, 1, 1)]
void csArr()
{
    buffer.Store(48, buffer.Load<float3[2]>(0));
}
// ARR: OpEntryPoint
// ARR: OpDecorate %_runtimearr_float ArrayStride 4
// ARR-NOT: OpTypeRuntimeArray %v3float
// ARR-NOT: OpLoad %v3float

// An 8-byte-element `double3` takes the structurally distinct path (natural stride 24, std430
// stride 32); at byte 24 (a nonzero multiple of the 24-byte natural stride) it must scalarize to a
// tight `double` runtime array (ArrayStride 8), reading indices 24/8=3,4,5 and storing 64/8=8,9,10.
[shader("compute")]
[numthreads(1, 1, 1)]
void csDouble3()
{
    buffer.Store(64, buffer.Load<double3>(24));
}
// DOUBLE3: OpEntryPoint
// DOUBLE3: OpDecorate %_runtimearr_double ArrayStride 8
// DOUBLE3-NOT: OpTypeRuntimeArray %v3double
// DOUBLE3: OpAccessChain %_ptr_StorageBuffer_double {{.*}} %uint_3
// DOUBLE3-NOT: OpLoad %v3double
// DOUBLE3: OpLoad %double
// DOUBLE3-NOT: OpLoad %v3double
// DOUBLE3: OpAccessChain %_ptr_StorageBuffer_double {{.*}} %uint_8
// DOUBLE3-NOT: OpLoad %v3double
// DOUBLE3-NOT: OpTypeRuntimeArray %v3double
