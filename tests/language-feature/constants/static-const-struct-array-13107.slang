// Regression test for shader-slang/slang#13107.
//
// A module-scope `static const` array of POD structs must be emitted as an
// immutable global table, exactly like a `static const` array of scalars. Before
// the fix it was reconstructed once per invocation from synthesized member-wise
// constructor (`$init`) calls inside every using function, because
// `isSimpleConstantType` rejected structs (so the table was force-inlined by
// `inlineGlobalConstantsForLegalization`) and because the constant elements were
// represented as `IRCall`s rather than `IRMakeStruct`. On CUDA/OptiX that
// per-invocation copy added a large local frame and (for big tables) hundreds of
// MiB at launch.
//
// The legalization pass that force-inlines non-simple global constants runs for
// every target whose `shouldLegalizeExistentialAndResourceTypes` is on — i.e.
// HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA and the CPU-kernel path — so the bug (and
// this fix) is not CUDA-specific; each target below reconstructed the table
// per-invocation before the fix. The `-NOT` lines assert the synthesized ctor is
// no longer used to rebuild the table.

//TEST:SIMPLE(filecheck=CUDA):   -target cuda  -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=PTX):    -target ptx   -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP):    -target cpp   -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=HLSL):   -target hlsl  -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL):   -target glsl  -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=SPIRV):  -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):  -target metal -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=WGSL):   -target wgsl  -entry computeMain -stage compute -line-directive-mode none

struct Record
{
    uint a;
    uint b;
}

static const Record records[2] = {{1, 2}, {3, 4}};

// Scalar-element control: this already stayed global before the fix; it must
// still be global (i.e. the fix does not regress the scalar path).
static const uint scalars[4] = {10, 20, 30, 40};

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    outBuf[0] = records[i].a + records[i].b + scalars[i];
}

// CUDA: __device__ static const FixedArray<Record_0, 2>{{.*}}records_0
// CUDA-NOT: Record{{.*}}init

// End-to-end through NVRTC: the table is a statically-initialized `.global`
// array holding the literal bytes ({1,2},{3,4} as little-endian u32s), with no
// per-invocation reconstruction. This run is skipped where the CUDA toolchain is
// unavailable.
// PTX: .global{{.*}}records_0{{.*}}1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4
// PTX-NOT: x24init

// CPP: static const FixedArray<Record_0, 2>{{.*}}records_0 = { { { 1U, 2U }, { 3U, 4U } } };
// CPP-NOT: Record{{.*}}init

// HLSL: static const Record_0{{.*}}records_0[int(2)] = { { 1U, 2U }, { 3U, 4U } };
// HLSL-NOT: Record{{.*}}init

// GLSL: const Record_0{{.*}}records_0[2] = Record_0[](Record_0(1U, 2U), Record_0(3U, 4U));
// GLSL-NOT: Record{{.*}}init

// The array is a genuine constant composite, not a value reconstructed at
// runtime with OpCompositeConstruct inside the entry point.
// SPIRV: OpConstantComposite %_arr_Record{{.*}}
// SPIRV-NOT: OpCompositeConstruct

// METAL: constant array<Record_0, int(2)> records_0 = { { { 1U, 2U }, { 3U, 4U } } };
// METAL-NOT: Record{{.*}}init

// WGSL: var<private> records_0 : array<Record_0, i32(2)> = array<Record_0, i32(2)>(
// WGSL-NOT: Record{{.*}}init
