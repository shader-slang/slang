//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=A NewExpr `new T(...)` passes its arguments to the class constructor, so the allocated instance carries the constructor-computed field value.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The allocation form takes an argument list, so `new T(args)` has to resolve
// to the class's constructor the same way `T(args)` does for a struct. The
// constructor here doubles its argument and the dispatch id keeps the value
// symbolic; the CHECK asserts the doubling appears in the stored expression,
// which is only there if the argument list reached the constructor.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

class Counter
{
    int v;
    __init(int start) { v = start * 2; }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Counter c = new Counter(4 + int(tid.x));
    outputBuffer[0] = c.v;
    // HLSL: outputBuffer{{[A-Za-z0-9_]*}}[int(0)] =
}
