//TEST:SIMPLE(filecheck=VP):-target spirv-asm -entry computeMain -stage compute -g0 -capability SPV_KHR_variable_pointers
//TEST:SIMPLE(filecheck=VPCOPY):-target spirv-asm -entry computeMain -stage compute -g0 -capability SPV_KHR_variable_pointers
//TEST:SIMPLE(filecheck=VPCOPY):-target spirv-asm -entry computeMain -stage compute -capability SPV_KHR_variable_pointers
//TEST:SIMPLE(filecheck=NOVP):-target spirv-asm -entry computeMain -stage compute -g0
//TEST:SIMPLE(filecheck=NOVP):-target spirv-asm -emit-spirv-via-glsl -entry computeMain -stage compute -g0 -capability SPV_KHR_variable_pointers
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute -capability SPV_KHR_variable_pointers
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute -capability SPV_KHR_variable_pointers

// Regression guard for https://github.com/shader-slang/slang/issues/10641.
//
// A `groupshared` parameter is a `Workgroup` pointer. On direct SPIR-V, a `[noinline]` callee keeps
// its call boundary when the target already enables `SPV_KHR_variable_pointers`: the parameter is
// specialized to a `Workgroup` pointer, the caller passes the shared variable or an access chain
// into it directly, and no per-thread copy is made, with or without debug info (VP, VPCOPY). Without the extension, and on
// GLSL, SPIR-V via GLSL and WGSL even with it, the callee is inlined (NOVP, GLSL, WGSL).

RWStructuredBuffer<uint> output;
groupshared uint shared[8];
groupshared uint rows[2][8];

[noinline]
void bump(groupshared uint a[8], uint i)
{
    a[i] = a[i] + 1;
}

[numthreads(8, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    shared[tid] = tid;
    rows[1][tid] = tid;
    GroupMemoryBarrierWithGroupSync();
    bump(shared, tid);
    bump(rows[1], tid);
    GroupMemoryBarrierWithGroupSync();
    output[tid] = shared[tid] + rows[1][tid];
}

// VP-DAG: OpCapability VariablePointers
// VP-DAG: OpExtension "SPV_KHR_variable_pointers"
// VP-DAG: %shared = OpVariable %_ptr_Workgroup__arr_uint_int_8 Workgroup
// VP-DAG: %[[ROW:[0-9]+]] = OpAccessChain %_ptr_Workgroup__arr_uint_int_8 %rows %int_1
// VP-DAG: OpFunctionCall %void %bump %shared
// VP-DAG: OpFunctionCall %void %bump %[[ROW]]
// VP-DAG: %a = OpFunctionParameter %_ptr_Workgroup__arr_uint_int_8

// VPCOPY-NOT: OpVariable %_ptr_Function

// NOVP-NOT: OpFunctionCall
// NOVP-NOT: VariablePointers

// GLSL-NOT: bump
// GLSL: void main(

// WGSL-NOT: fn bump
// WGSL: fn computeMain(
