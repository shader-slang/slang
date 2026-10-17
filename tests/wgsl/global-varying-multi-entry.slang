// Regression test for shader-slang/slang#13687.
//
// A module-scope `in` varying (here `gThreadId`, plus the core module's `__builtinWaveLaneCount`
// behind `WaveGetLaneCount()`) read by callees shared by two entry points must become a
// parameter of, and be stored into the private global by, *every* entry point that reaches it
// when both entry points are emitted into one module, not only the first one in module order.

//TEST:SIMPLE(filecheck=WGSLAB): -target wgsl -whole-program
//TEST:SIMPLE(filecheck=WGSLAB): -target wgsl -whole-program -entry mainA -stage compute -entry mainB -stage compute
//TEST:SIMPLE(filecheck=WGSLBA): -target wgsl -whole-program -entry mainB -stage compute -entry mainA -stage compute
//TEST:SIMPLE(filecheck=METALAB): -target metal -line-directive-mode none
//TEST:SIMPLE(filecheck=METALAB): -target metal -line-directive-mode none -entry mainA -stage compute -entry mainB -stage compute
//TEST:SIMPLE(filecheck=METALBA): -target metal -line-directive-mode none -entry mainB -stage compute -entry mainA -stage compute

in uint3 gThreadId : SV_DispatchThreadID;
RWStructuredBuffer<uint> o;

uint laneCount() { return WaveGetLaneCount(); }
uint threadIndex() { return gThreadId.x; }

// WGSLAB: fn mainA(@builtin(global_invocation_id) [[A_ID:[A-Za-z0-9_]+]] : vec3<u32>, @builtin(subgroup_size) [[A_LANES:[A-Za-z0-9_]+]] : u32)
// WGSLAB-NEXT: {
// WGSLAB-NEXT: = [[A_ID]];
// WGSLAB-NEXT: = [[A_LANES]];
// WGSLAB: fn mainB(@builtin(global_invocation_id) [[B_ID:[A-Za-z0-9_]+]] : vec3<u32>, @builtin(subgroup_size) [[B_LANES:[A-Za-z0-9_]+]] : u32)
// WGSLAB-NEXT: {
// WGSLAB-NEXT: = [[B_ID]];
// WGSLAB-NEXT: = [[B_LANES]];

// WGSLBA: fn mainB(@builtin(global_invocation_id) [[B_ID:[A-Za-z0-9_]+]] : vec3<u32>, @builtin(subgroup_size) [[B_LANES:[A-Za-z0-9_]+]] : u32)
// WGSLBA-NEXT: {
// WGSLBA-NEXT: = [[B_ID]];
// WGSLBA-NEXT: = [[B_LANES]];
// WGSLBA: fn mainA(@builtin(global_invocation_id) [[A_ID:[A-Za-z0-9_]+]] : vec3<u32>, @builtin(subgroup_size) [[A_LANES:[A-Za-z0-9_]+]] : u32)
// WGSLBA-NEXT: {
// WGSLBA-NEXT: = [[A_ID]];
// WGSLBA-NEXT: = [[A_LANES]];

// METALAB: void mainA(uint3 [[A_ID:[A-Za-z0-9_]+]] {{\[\[}}thread_position_in_grid{{\]\]}}, uint [[A_LANES:[A-Za-z0-9_]+]] {{\[\[}}threads_per_simdgroup{{\]\]}}
// METALAB: = [[A_ID]];
// METALAB: = [[A_LANES]];
// METALAB: void mainB(uint3 [[B_ID:[A-Za-z0-9_]+]] {{\[\[}}thread_position_in_grid{{\]\]}}, uint [[B_LANES:[A-Za-z0-9_]+]] {{\[\[}}threads_per_simdgroup{{\]\]}}
// METALAB: = [[B_ID]];
// METALAB: = [[B_LANES]];

// METALBA: void mainB(uint3 [[B_ID:[A-Za-z0-9_]+]] {{\[\[}}thread_position_in_grid{{\]\]}}, uint [[B_LANES:[A-Za-z0-9_]+]] {{\[\[}}threads_per_simdgroup{{\]\]}}
// METALBA: = [[B_ID]];
// METALBA: = [[B_LANES]];
// METALBA: void mainA(uint3 [[A_ID:[A-Za-z0-9_]+]] {{\[\[}}thread_position_in_grid{{\]\]}}, uint [[A_LANES:[A-Za-z0-9_]+]] {{\[\[}}threads_per_simdgroup{{\]\]}}
// METALBA: = [[A_ID]];
// METALBA: = [[A_LANES]];

[shader("compute")]
[numthreads(64, 1, 1)]
void mainA()
{
    o[threadIndex()] = laneCount();
}

[shader("compute")]
[numthreads(32, 1, 1)]
void mainB()
{
    o[threadIndex() + 64] = laneCount();
}
