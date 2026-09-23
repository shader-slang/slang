//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=CHECKCUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECKHLSL):-target hlsl -entry computeMain -stage compute

// Regression for #13233. Forward-mode differentiation of a function whose parameter is an
// `IDifferentiable` struct imported from another module used to mistype the derivative's
// parameter as the primal struct with a hardcoded zero tangent -- but only when the struct
// combined an explicit `__init()` with a `no_diff` field.
//
// The differentiable-type association for the imported `Value` is emitted onto this
// module's `[import]` declaration, while link selects the defining module's `[export]`
// (which lacks the association, because its explicit `__init()` means it never
// differentiates `Value`) as the canonical inst. Before the fix the association was not
// carried across, so `fwd_diff(metric)` fell back to a non-differentiable parameter: the
// interpreter returned a zero tangent, and the CUDA derivative took the primal `Value`
// instead of `DifferentialPair<Value>`.

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
