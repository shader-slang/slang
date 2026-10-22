// Regression test for https://github.com/shader-slang/slang/issues/13841
// A forward derivative is built from copies of the primal function's parameters, which carry
// `DebugVarStorage` decorations naming the primal function's debug variables. The derivative
// rebinds them to its own debug variables, so its preserved `__constref` parameter `bias` is
// declared on the parameter, like the primal function's. The trivial derivative synthesized for
// `[TreatAsDifferentiable]` `bumpCounter` copies parameters too but has no debug variables of
// its own, so it drops the decorations; both must compile at -g2 under -validate-ir.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -g2 -O0 -validate-ir

RWStructuredBuffer<float> outputBuffer;

struct NonDiff
{
    float a;
}

[Differentiable]
float scaleBy(__constref NonDiff k, float x, no_diff __constref float bias)
{
    return x * k.a + bias;
}

[TreatAsDifferentiable]
void bumpCounter(no_diff inout int counter, float x)
{
    counter += 1;
}

[Differentiable]
float useCounter(float x)
{
    int n = 2;
    bumpCounter(n, x);
    return float(n) * x;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    NonDiff k = { 10.0 };
    outputBuffer[0] = __fwd_diff(scaleBy)(k, diffPair(4.0, 1.0), 1.0).d;
    outputBuffer[1] = __fwd_diff(useCounter)(diffPair(0.5, 1.0)).d;
    outputBuffer[2] = scaleBy(k, 4.0, 1.0);
}

// CHECK: OpEntryPoint
// CHECK-NOT: OpName %_dbgvar_bias
// CHECK-LABEL: %s_fwd_scaleBy = OpFunction
// CHECK: [[FWD_BIAS_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// CHECK: DebugDeclare {{%[A-Za-z_0-9]+}} [[FWD_BIAS_PARAM]] {{%[0-9]+}}{{$}}
// CHECK-LABEL: %scaleBy = OpFunction
// CHECK: [[BIAS_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// CHECK: DebugDeclare {{%[A-Za-z_0-9]+}} [[BIAS_PARAM]] {{%[0-9]+}}{{$}}
