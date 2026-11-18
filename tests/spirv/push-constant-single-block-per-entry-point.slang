// Each entry point below statically uses at most one push-constant block, so
// the output is valid and the check for more than one push-constant block per
// entry point (shader-slang/slang#13713) must not fire.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -profile spirv_1_5
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry onlyEntryPointUniforms -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry onlyGlobalBlock -stage compute
//TEST:SIMPLE(filecheck=NOERR): -target glsl -entry onlyGlobalBlock -stage compute -preserve-params
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry deadBranch -stage compute
//TEST:SIMPLE(filecheck=FRAG): -target spirv -profile spirv_1_5 -entry discardThenRead -stage fragment
//TEST:SIMPLE(filecheck=FRAG): -target spirv -profile spirv_1_5 -entry discardThenRead -stage fragment -O0

struct PC
{
    float4 a;
}

struct PC2
{
    float4 b;
}

[[vk::push_constant]]
uniform PC push;

[[vk::push_constant]]
uniform PC2 push2;

RWStructuredBuffer<float> outBuf;

static const bool kUseSecond = false;

// Only the entry point's own `uniform` parameter block is used.
[shader("compute")]
[numthreads(4, 1, 1)]
void onlyEntryPointUniforms(uint3 tid: SV_DispatchThreadID, uniform float k)
{
    outBuf[tid.x] = k;
}

// Only the global block is used; the `uniform` parameter is never read.
[shader("compute")]
[numthreads(4, 1, 1)]
void onlyGlobalBlock(uint3 tid: SV_DispatchThreadID, uniform float unused)
{
    outBuf[tid.x] = push.a.x;
}

// The read of the second block is in a branch that is removed as dead code.
[shader("compute")]
[numthreads(4, 1, 1)]
void deadBranch(uint3 tid: SV_DispatchThreadID)
{
    float v = push.a.x;
    if (kUseSecond)
        v += push2.b.y;
    outBuf[tid.x] = v;
}

// `onlyGlobalBlock` and `secondGlobalBlock` are two entry points that each use a different
// global block.
[shader("compute")]
[numthreads(4, 1, 1)]
void secondGlobalBlock(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = push2.b.x;
}

// Before SPIR-V 1.6, `discard` becomes `OpKill`, a block terminator, so the
// read of `push` after it is unreachable and removed before the entry point's
// interface is formed.
[shader("fragment")]
float4 discardThenRead(float4 pos: SV_Position, uniform float k) : SV_Target
{
    float4 c = float4(k);
    if (pos.x > 10)
    {
        discard;
        c += push.a;
    }
    return c;
}

// FRAG-NOT: E39033
// FRAG: OpEntryPoint Fragment %discardThenRead

// With `-preserve-params` the unused blocks stay in the module, but only the
// block that `onlyGlobalBlock` reads is declared.
// NOERR-NOT: E39033
// NOERR: layout(push_constant)
// NOERR-NEXT: uniform block_PC_
// NOERR-NOT: layout(push_constant)

// SPIRV-NOT: E39033
// SPIRV-NOT: error[
// SPIRV-DAG: OpEntryPoint GLCompute %onlyEntryPointUniforms
// SPIRV-DAG: OpEntryPoint GLCompute %onlyGlobalBlock
// SPIRV-DAG: OpEntryPoint GLCompute %deadBranch
// SPIRV-DAG: OpEntryPoint GLCompute %secondGlobalBlock

// GLSL-COUNT-1: layout(push_constant)
// GLSL-NOT: layout(push_constant)
