// Regression test for the `CallShader` caller side of the empty-callable-payload erasure, across
// every capability-allowed Vulkan/D3D/CUDA target. An empty callable-data payload passed to `CallShader`
// must survive type legalization instead of being dropped/erased:
//   - D3D/HLSL (#12718): the emitted HLSL must still pass the payload as `CallShader`'s 2nd arg
//     (the bug emitted a one-argument `CallShader(0U)` that DXC rejects).
//   - SPIR-V (#12731): the `[__vulkanCallablePayload]` global must keep a real Callable Data
//     variable for `OpExecuteCallableKHR` (the bug aborted with "non-simple operand(s)!").
//   - GLSL (same path): the same global feeds `executeCallableEXT`; the bug aborted the same way.
//   - CUDA (#13106): `CallShader`'s CUDA arm is the fixed-arity `optixDirectCall<void>($0, $1)`, so
//     the dropped payload argument used to abort intrinsic expansion (`slang-intrinsic-expand.cpp`);
//     the padded payload must be passed as the real second argument.

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

// The empty payload is padded with a dummy field, and the padded payload local is passed as
// `CallShader`'s second argument, rather than being dropped (the bug emitted a one-argument
// `CallShader(0U)`).
// HLSL-DAG: int _slang_dummy{{.*}};
// HLSL: EmptyData{{[A-Za-z0-9_]*}} [[payload:[A-Za-z0-9_]+]];
// HLSL: CallShader(0U, [[payload]]);

// DXC accepts the two-argument call: rejecting the one-argument form was the #12718 symptom, so a
// successful DXIL compile is the real end-to-end signal.
// DXIL: define void @

// SPIR-V keeps a real Callable Data variable and passes it as `OpExecuteCallableKHR`'s operand,
// instead of aborting type legalization on a `none`-typed operand.
// SPIRV: %[[payload:[0-9A-Za-z_]+]] = OpVariable %{{[0-9A-Za-z_]+}} CallableDataKHR
// SPIRV: OpExecuteCallableKHR %{{[0-9A-Za-z_]+}} %[[payload]]

// GLSL compiles (instead of aborting): the padded payload survives as a real `callableDataEXT`
// global and the callable dispatch is emitted.
// GLSL-DAG: _slang_dummy
// GLSL-DAG: callableDataEXT
// GLSL-DAG: executeCallableEXT

// CUDA behaves the same way: the padded payload local survives and is passed as
// `optixDirectCall`'s second argument, rather than being dropped (which left
// `optixDirectCall<void>($0)` and aborted the fixed-arity `$1` expansion). The `__raygen__rgen`
// entry point compiling is the positive control.
// CUDA: int _slang_dummy
// CUDA: __raygen__rgen()
// CUDA: EmptyData{{[A-Za-z0-9_]*}} [[cudaPayload:[A-Za-z0-9_]+]];
// CUDA: optixDirectCall<void>((0U), (&[[cudaPayload]]));
