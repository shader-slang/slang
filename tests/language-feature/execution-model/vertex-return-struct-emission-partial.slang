// Regression test for shader-slang/slang#12756: a vertex entry point
// returns an output struct in which the `uv` field is never assigned.
//
// Contract pinned here:
//   * `uv` stays declared in the entry-point interface (it keeps Location 1
//     and its Output variable).
//   * The unwritten field is not read from the returned struct value: on
//     SPIR-V no store into `uv` is emitted at -O1; on GLSL the copy-out is
//     not `= <ret>.uv`.
//   * At -O0 (where the redundant undef store is not yet elided) the fix is
//     pinned directly at the changed IR layer: the `uv` store's operand is an
//     `OpUndef`, not an `OpCompositeExtract`/`OpLoad` of the returned local.
//     This -O0 check is spirv-opt-independent, unlike the SPIR-V -O1 check
//     whose fail-on-buggy behavior relies on spirv-opt.

//TEST:SIMPLE(filecheck=CHECK_SPV):    -target spirv-asm -entry vertMain -stage vertex -emit-spirv-directly -O1
//TEST:SIMPLE(filecheck=CHECK_SPV_O0): -target spirv-asm -entry vertMain -stage vertex -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=CHECK_GLSL):   -target glsl      -entry vertMain -stage vertex -O1

struct VOut
{
    float3 color    : COLOR;
    float2 uv       : TEXCOORD0;
    float4 position : SV_Position;
};

[shader("vertex")]
VOut vertMain(float3 pos : POSITION, float3 inColor : COLOR)
{
    VOut o;
    o.color    = inColor * 0.5;
    o.position = float4(pos, 1.0); // o.uv is intentionally never assigned
    return o;
}

// The `uv` varying output stays declared: it is in the OpEntryPoint
// interface, keeps Location 1, and has its Output variable, exactly like
// the written varyings.
// CHECK_SPV-DAG: OpEntryPoint Vertex %vertMain "main" {{.*}}%entryPointParam_vertMain_uv
// CHECK_SPV-DAG: OpDecorate %entryPointParam_vertMain_uv Location 1
// CHECK_SPV-DAG: %entryPointParam_vertMain_uv = OpVariable %_ptr_Output_v2float Output

// The varying stores are emitted in struct-field order (color, uv,
// position). Anchoring on the color and position/gl_Position stores and
// forbidding a `uv` store strictly between them asserts that nothing is
// written into the never-written `uv` output. (The buggy compiler emits
// `OpStore %entryPointParam_vertMain_uv` in exactly this region, so the
// CHECK-NOT must be bounded here to catch it.)
// CHECK_SPV: OpStore %entryPointParam_vertMain_color
// CHECK_SPV-NOT: OpStore %entryPointParam_vertMain_uv
// CHECK_SPV: OpStore %gl_Position

// GLSL: the `uv` output interface variable is still declared. The written
// varyings are copied out; the unwritten `uv` field must not be read from
// the returned struct value between them.
// CHECK_GLSL-DAG: out vec2 entryPointParam_vertMain_uv
// CHECK_GLSL: entryPointParam_vertMain_color{{.*}} = {{.*}}.color
// CHECK_GLSL-NOT: entryPointParam_vertMain_uv{{.*}} = {{.*}}.uv
// CHECK_GLSL: gl_Position = {{.*}}.position

// At -O0 the store into `uv` survives (it is not yet elided), so pin its
// operand directly: it must be an `OpUndef`, i.e. a canonical undefined value,
// not a read of the never-written field of the returned struct. The buggy
// compiler feeds the store from `OpCompositeExtract`/`OpLoad` of the returned
// local, so this fails on the unfixed compiler at the exact IR layer changed.
// CHECK_SPV_O0-DAG: %entryPointParam_vertMain_uv = OpVariable %_ptr_Output_v2float Output
// CHECK_SPV_O0: [[UV_UNDEF:%[0-9]+]] = OpUndef %v2float
// CHECK_SPV_O0: OpStore %entryPointParam_vertMain_uv [[UV_UNDEF]]
