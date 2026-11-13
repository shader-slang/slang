//TEST:SIMPLE(filecheck=CHECK):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain -validate-ir
//TEST:SIMPLE(filecheck=FREC):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain -validate-ir

// Regression guard for shader-slang/slang#13238 when the differentiated body also contains debug
// scopes that the inliner copied without an inlinedAt operand.
//
// g is [__unsafeForceInlineEarly], so it is inlined into f before differentiation. The call to g sits
// in a branch of the conditional expression, a block with no DebugLine before the call, so the
// inliner has no call-site location for an inline record and copies g's DebugScopes into f without an
// inlinedAt operand. f's derivatives inherit those scopes along with the scopes of f's own tree.
// copyDebugInfo re-roots only the inherited tree that its caller names, so each derivative's own code
// runs in the derivative's record and no stale record named "f" survives. The copied g scopes keep
// naming g in the derivatives, as they do in f; giving them an inline record is an inliner change.
//
// fwd_diff(f) keeps s_fwd_f alive until SPIR-V emission. Without it, the only derivatives whose
// bodies inherit the mixed scopes are removed before emission, and these checks could not see them.

RWStructuredBuffer<float> outputBuffer;

[__unsafeForceInlineEarly]
float g(float x)
{
    float r = x;
    if (x > 0.0)
    {
        r = r * 2.0;
    }
    return r;
}

[Differentiable]
float f(float x)
{
    return x > 1.0 ? g(x) : x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    DifferentialPair<float> dp = diffPair(2.0, 0.0);
    bwd_diff(f)(dp, 1.0);
    outputBuffer[tid.x] = dp.d + f(3.0) + fwd_diff(f)(diffPair(2.0, 1.0)).d;
}

// Each surviving function owns a DebugFunction record bound to its own body.
// CHECK-DAG: [[BWD_STR:%[0-9]+]] = OpString "s_bwdProp_f"
// CHECK-DAG: [[FWD_STR:%[0-9]+]] = OpString "s_fwd_f"
// CHECK-DAG: [[BWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[BWD_STR]] %
// CHECK-DAG: [[FWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[FWD_STR]] %
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[BWD_REC]] %s_bwdProp_f
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[FWD_REC]] %s_fwd_f

// Exactly one record is named "f", and it is bound to f's own body. A derivative whose scopes were
// not re-rooted would leave a second "f" record, with no definition, for its code to run in.
// FREC: [[F_STR:%[0-9]+]] = OpString "f"
// FREC: [[F_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[F_STR]] %
// FREC-NOT: OpExtInst %void %{{[0-9]+}} DebugFunction [[F_STR]] %
// FREC: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[F_REC]] %f{{$}}
