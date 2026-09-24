// Regression test for the `CallShader` caller side of empty-callable-payload legalization across
// every capability-allowed Vulkan/D3D/CUDA target. D3D and Vulkan require a physical callable-data
// object, while CUDA/OptiX permits the empty payload to have no physical argument:
//   - D3D/HLSL (#12718): the emitted HLSL must still pass the payload as `CallShader`'s 2nd arg
//     (the bug emitted a one-argument `CallShader(0U)` that DXC rejects).
//   - SPIR-V (#12731): the `[__vulkanCallablePayload]` global must keep a real Callable Data
//     variable for `OpExecuteCallableKHR` (the bug aborted with "non-simple operand(s)!").
//   - GLSL (same path): the same global feeds `executeCallableEXT`; the bug aborted the same way.
//   - CUDA (#13106): the empty payload is erased, and the variadic OptiX intrinsic expansion emits
//     only the shader index instead of trying to reference a missing fixed `$1` operand.

//TEST:SIMPLE(filecheck=HLSL): -stage raygeneration -entry rgen -target hlsl -profile sm_6_7
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry rgen -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=SPIRV): -stage raygeneration -entry rgen -target spirv
//TEST:SIMPLE(filecheck=GLSL): -stage raygeneration -entry rgen -target glsl
//TEST:SIMPLE(filecheck=CUDA): -stage raygeneration -entry rgen -target cuda

struct EmptyData
{
}

[shader("raygeneration")]
void rgen()
{
    EmptyData d;
    CallShader(0, d);
}

// A separate one-int carrier is passed as CallShader's second argument. The logical EmptyData
// type remains empty; only this ABI boundary requires storage. The bug emitted CallShader(0U).
// HLSL: struct [[carrier:[A-Za-z0-9_]+]]
// HLSL-NEXT: {
// HLSL-NEXT: int _slang_dummy{{.*}};
// HLSL-NEXT: };
// HLSL: [[carrier]] [[payload:[A-Za-z0-9_]+]];
// HLSL: CallShader(0U, [[payload]]);

// DXC accepts the two-argument call: rejecting the one-argument form was the #12718 symptom, so a
// successful DXIL compile is the real end-to-end signal.
// DXIL: define void @

// SPIR-V keeps a real Callable Data variable and passes it as `OpExecuteCallableKHR`'s operand,
// instead of aborting type legalization on a `none`-typed operand.
// SPIRV: %[[payload:[0-9A-Za-z_]+]] = OpVariable %{{[0-9A-Za-z_]+}} CallableDataKHR
// SPIRV: OpExecuteCallableKHR %{{[0-9A-Za-z_]+}} %[[payload]]

// GLSL compiles (instead of aborting): the physical carrier survives as a real `callableDataEXT`
// global and the callable dispatch is emitted.
// GLSL-DAG: _slang_dummy
// GLSL-DAG: callableDataEXT
// GLSL-DAG: executeCallableEXT

// OptiX direct calls are variadic, so CUDA keeps no artificial payload storage or argument. The
// old fixed `$1` intrinsic expansion aborted after type legalization erased the empty payload.
// CUDA-NOT: _slang_dummy
// CUDA-LABEL: void __raygen__rgen()
// CUDA: optixDirectCall<void>(0U);
