// Regression test for https://github.com/shader-slang/slang/issues/13841
// Autodiff builds derivative functions from copies of the primal function's parameters, made
// before the debug variables those parameters hold. Each derivative repoints the copied
// `DebugVarStorage` decorations at its own debug variables or drops them:
// - the forward derivative of `scaleBy` declares its kept `__constref` parameter `bias` on the
//   parameter, like the primal function does;
// - the forward derivative of `countAndScale` reads and writes a temporary for its mutable
//   parameter `counter` and writes the parameter only on return, so `counter` is not declared on
//   the parameter;
// - the reverse derivative of `offsetBy` and the trivial derivative synthesized for
//   `[TreatAsDifferentiable]` `bumpCounter` drop the decorations they cannot repoint, and must
//   compile under -validate-ir.

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

[Differentiable]
float countAndScale(no_diff inout int counter, float x)
{
    counter += 1;
    return x * float(counter);
}

[Differentiable]
float offsetBy(float x, no_diff __constref float bias)
{
    return x * x + bias;
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
    int c = 1;
    outputBuffer[3] = __fwd_diff(countAndScale)(c, diffPair(2.0, 1.0)).d;
    float bias = outputBuffer[5];
    var dpx = diffPair(outputBuffer[6], 0.0);
    __bwd_diff(offsetBy)(dpx, bias, 1.0);
    outputBuffer[4] = dpx.d;
}

// CHECK: OpEntryPoint
// CHECK-LABEL: %s_fwd_scaleBy = OpFunction
// CHECK: [[FWD_BIAS_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// CHECK: DebugDeclare {{%[A-Za-z_0-9]+}} [[FWD_BIAS_PARAM]] {{%[0-9]+}}{{$}}
// CHECK-LABEL: %scaleBy = OpFunction
// CHECK: [[BIAS_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// CHECK: DebugDeclare {{%[A-Za-z_0-9]+}} [[BIAS_PARAM]] {{%[0-9]+}}{{$}}
// CHECK-LABEL: %s_fwd_countAndScale = OpFunction
// CHECK: [[COUNTER_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_int
// CHECK-NOT: DebugDeclare {{%[A-Za-z_0-9]+}} [[COUNTER_PARAM]]
// CHECK: OpFunctionEnd
