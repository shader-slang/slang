//TEST:SIMPLE(filecheck=A): -target wgsl -entry mainA -stage compute
//TEST:SIMPLE(filecheck=B): -target wgsl -entry mainB -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry mainA -stage compute
//TEST:SIMPLE(filecheck=C): -target wgsl -entry mainC -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry mainB -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry mainC -stage compute
//TEST:SIMPLE(filecheck=WHOLE): -target wgsl -whole-program
//TEST:SIMPLE(filecheck=WHOLETINT): -target wgsl-spirv-asm -whole-program

// Entry-point inputs that callees read are passed to them as parameters (see
// builtin-input-uniformity.slang). This covers the shapes of that rewrite:
// - a callee with no parameters of its own (`laneCount`);
// - a chain where only the innermost function reads the input (mainA -> middle -> leaf ->
//   laneCount);
// - an input that is not a subgroup built-in (`SV_DispatchThreadID` through a module-scope
//   `in` variable, read by `threadIndex`);
// - a module-scope initializer (`g = seed()`) that the entry point calls before it stores the
//   inputs; it does not read them, so the rewrite still applies;
// - a module-scope initializer that reads an input (`gLanes = WaveGetLaneCount()`, mainC). It runs
//   before the entry point stores the input, so it gets the entry point's parameter.
// The new parameters take the name of the input.

// A-NOT: var<private> {{.*}} : u32;{{$}}
// A-NOT: var<private> {{.*}} : vec3<u32>;
// A: fn threadIndex_{{[0-9]+}}( gThreadId_{{[0-9]+}} : vec3<u32>) -> u32
// A: var<private> g_{{[0-9]+}} : u32;
// A: fn laneCount_{{[0-9]+}}( {{[A-Za-z0-9_]+}} : u32) -> u32
// A: fn leaf_{{[0-9]+}}( v_{{[0-9]+}} : u32,  {{[A-Za-z0-9_]+}} : u32) -> u32
// A: fn middle_{{[0-9]+}}( v_{{[0-9]+}} : u32,  [[MIDDLE_SIZE:[A-Za-z0-9_]+]] : u32) -> u32
// A: leaf_{{[0-9]+}}({{.*}}, [[MIDDLE_SIZE]])
// A: fn mainA(@builtin(global_invocation_id) [[TID:[A-Za-z0-9_]+]] : vec3<u32>, @builtin(subgroup_size) [[SIZE:[A-Za-z0-9_]+]] : u32)
// A: g_{{[0-9]+}} = 
// A: threadIndex_{{[0-9]+}}([[TID]])
// A: middle_{{[0-9]+}}({{.*}}, [[SIZE]])

// B-NOT: var<private>
// B: fn leaf_{{[0-9]+}}( v_{{[0-9]+}} : u32,  {{[A-Za-z0-9_]+}} : u32) -> u32
// B: fn mainB(@builtin(global_invocation_id) [[TID:[A-Za-z0-9_]+]] : vec3<u32>, @builtin(subgroup_size) [[SIZE:[A-Za-z0-9_]+]] : u32)
// B: threadIndex_{{[0-9]+}}([[TID]])
// B: leaf_{{[0-9]+}}({{.*}}, [[SIZE]])

// C-NOT: var<private> {{.*}} : u32;{{$}}
// C: fn [[INIT:[A-Za-z0-9_]+]]( [[INIT_SIZE:_builtinWaveLaneCount_[0-9]+]] : u32) -> u32
// C-NEXT: {
// C: return [[INIT_SIZE]];
// C: var<private> gLanes_{{[0-9]+}} : u32;
// C: fn mainC({{.*}}@builtin(subgroup_size) [[SIZE:[A-Za-z0-9_]+]] : u32)
// C-NEXT: {
// C-NEXT: [[INIT]]([[SIZE]]);
// C: leaf_{{[0-9]+}}({{.*}}, [[SIZE]])

// When several entry points are compiled into one module and only one of them (mainA) declares
// the inputs, the variables are left as they are.
// WHOLE: var<private> {{.*}} : vec3<u32>;
// WHOLE: fn threadIndex_{{[0-9]+}}() -> u32
// WHOLE: fn mainA(
// WHOLE: fn mainB()
// WHOLE: fn mainC()
// WHOLETINT-DAG: OpEntryPoint GLCompute %mainA
// WHOLETINT-DAG: OpEntryPoint GLCompute %mainB
// WHOLETINT-DAG: OpEntryPoint GLCompute %mainC

// TINT: OpGroupNonUniformIAdd

in uint3 gThreadId : SV_DispatchThreadID;

uint seed() { return 7; }
static uint g = seed();

RWStructuredBuffer<uint> outputBuffer;

uint laneCount() { return WaveGetLaneCount(); }

uint leaf(uint v)
{
    uint r = v;
    for (uint i = 0; i < 64 / laneCount(); i++)
        r = WaveActiveSum(r);
    return r;
}

uint middle(uint v) { return leaf(v + 1); }

uint threadIndex() { return gThreadId.x; }

[shader("compute")]
[numthreads(64, 1, 1)]
void mainA()
{
    outputBuffer[threadIndex()] = middle(outputBuffer[threadIndex()] + g);
}

[shader("compute")]
[numthreads(32, 1, 1)]
void mainB()
{
    outputBuffer[threadIndex() + 64] = leaf(threadIndex());
}

static uint gLanes = WaveGetLaneCount();

uint lanes() { return gLanes; }

[shader("compute")]
[numthreads(64, 1, 1)]
void mainC()
{
    outputBuffer[threadIndex() + 128] = leaf(lanes());
}
