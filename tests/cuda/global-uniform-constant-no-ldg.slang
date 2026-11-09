//TEST:SIMPLE(filecheck=SCALAR): -target cuda -entry raygenMain -stage raygeneration
//TEST:SIMPLE(filecheck=SCALAR): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=STRUCT): -target cuda -entry structMain -stage compute
//TEST:SIMPLE(filecheck=ARRAY): -target cuda -entry arrayMain -stage compute

// Regression test for shader-slang/slang#13088: the CUDA immutable-load pass must not
// lower a load rooted at the synthesized `__constant__` global uniform parameter group
// (`SLANG_globalParams`) to `__ldg`. `__ldg` lowers to PTX `ld.global.nc`, which the PTX
// ISA requires to address CUDA global memory; `SLANG_globalParams` lives in CUDA
// `__constant__` memory instead, so an `__ldg` there is illegal codegen even though the
// value never changes. A pointer *stored inside* that group (e.g. a genuine
// `ConstantBuffer<>`/`StructuredBuffer<>` field) is unaffected: reading through it loads
// the pointer out of `SLANG_globalParams` first, and that load — not the group itself —
// is the root of the subsequent access, so it correctly keeps `__ldg`. See the positive
// companion `constant-buffer-ldg.slang`.
//
// `CHECK-NOT` on both sides of the anchor: a regressed `__ldg` can land before the
// anchor (in a `slang_ldg` helper emitted earlier) or after it (inline). The anchor is
// `SLANG_globalParams`, the fixed symbol name the emitter always uses for this group
// (see `CUDASourceEmitter::emitParameterGroupImpl`), rather than a user-name-derived,
// mangling-counter-suffixed identifier like `gValue_0`.

RWStructuredBuffer<uint> outBuf;

uniform uint gValue;

//SCALAR-NOT: __ldg
//SCALAR: SLANG_globalParams
//SCALAR-NOT: __ldg

[shader("raygeneration")]
void raygenMain()
{
    outBuf[0] = gValue;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = gValue;
}

struct Nested
{
    uint a;
    float b;
};

uniform Nested gStruct;

//STRUCT-NOT: __ldg
//STRUCT: SLANG_globalParams
//STRUCT-NOT: __ldg

[shader("compute")]
[numthreads(1, 1, 1)]
void structMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = gStruct.a;
}

uniform uint gArray[4];

//ARRAY-NOT: __ldg
//ARRAY: SLANG_globalParams
//ARRAY-NOT: __ldg

[shader("compute")]
[numthreads(1, 1, 1)]
void arrayMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = gArray[tid.x % 4];
}
