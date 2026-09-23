//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=CHECKCUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECKHLSL):-target hlsl -entry computeMain -stage compute

// Regression for #13233: forward-mode differentiation of a function taking an imported
// `IDifferentiable` struct that combines an explicit `__init()` with a `no_diff` field.
//
// The struct's `DifferentialPairType` association is recorded on this module's `[import]`
// declaration of `Value`, since it is this module that differentiates `Value`; the defining
// module's `[export]` carries no such association (see the -lib file). Linking must preserve
// that association onto the canonical `Value`, so `fwd_diff(metric)` types the parameter as
// `DifferentialPair<Value>` with a real tangent rather than the primal `Value` with a zero
// tangent -- checked here on the interpreter (value) and CUDA/HLSL (emitted signature).

import "imported-nodiff-field-explicit-ctor-lib";

[Differentiable]
float metric(Value v)
{
    return v.x * 2.0;
}

void main()
{
    Value v;                     // x = 0
    var dp = diffPair(v, {1.0}); // tangent dx = 1
    var r = fwd_diff(metric)(dp);

    // Primal metric(v) = 0; tangent 2 * dv.x = 2.
    printf("%f %f\n", r.p, r.d);
    // CHECK: 0.000000 2.000000
}

// The forward-derivative of `metric` must take a `DifferentialPair` of the imported
// struct, not the primal `Value`.
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    Value v;
    var dp = diffPair(v, {1.0});
    outputBuffer[0] = fwd_diff(metric)(dp).d;
    // CHECKCUDA: s_fwd_metric{{.*}}(DiffPair_Value
    // CHECKHLSL: s_fwd_metric{{.*}}(DiffPair_Value
}
