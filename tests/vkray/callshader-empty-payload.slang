// Regression test for the `CallShader` caller side of
// https://github.com/shader-slang/slang/issues/12718 (D3D drops the payload argument) and
// https://github.com/shader-slang/slang/issues/12731 (SPIR-V aborts with "non-simple operand(s)!").
// An empty callable-data payload passed to `CallShader` must survive type legalization: on D3D the
// emitted HLSL must still pass the payload as `CallShader`'s second argument, and on SPIR-V the
// `[__vulkanCallablePayload]` global must keep a real Callable Data variable for
// `OpExecuteCallableKHR`.

//TEST:SIMPLE(filecheck=HLSL): -stage raygeneration -entry rgen -target hlsl -profile sm_6_7
//TEST:SIMPLE(filecheck=SPIRV): -stage raygeneration -entry rgen -target spirv

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

// SPIR-V keeps a real Callable Data variable and passes it as `OpExecuteCallableKHR`'s operand,
// instead of aborting type legalization on a `none`-typed operand.
// SPIRV: %[[payload:[0-9A-Za-z_]+]] = OpVariable %{{[0-9A-Za-z_]+}} CallableDataKHR
// SPIRV: OpExecuteCallableKHR %{{[0-9A-Za-z_]+}} %[[payload]]
