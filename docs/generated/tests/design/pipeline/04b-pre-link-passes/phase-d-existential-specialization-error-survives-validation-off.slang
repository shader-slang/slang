//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#per-translation-unit-predicates
//META: doc_section_digest=b8fbb9bb13768930ec6add0d3beb0867d01ef63afa7e06e8b99a7c850b497445
//META: purpose=D8 only marks the specialize instruction, so the existential-specialization error E33180 is still reported when non-essential validation is disabled and D8 does not run.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that D8 is not the step a user's diagnostic depends on. The
// error for specializing a generic with an existential type is raised later
// by the post-link specialization pass, which recognizes the shape by
// scanning bodies when D8's decoration is absent. The test compiles one
// program twice with the same annotations: once normally, and once with
// non-essential validation switched off, which is the toggle that skips D8.
// Both runs are required to report E33180 at the same caret, which is what
// shows the diagnostic does not come from D8.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain -disable-non-essential-validations

interface IFoo
{
    float calc(float x);
}

struct A : IFoo
{
    float calc(float x) { return x * x; }
}

struct B : IFoo
{
    float calc(float x) { return x * 2; }
}

float genericFunc<T : IFoo>(T obj, float x)
{
    return obj.calc(x);
}

IFoo makeInterface(uint id)
{
    if (id == 0)
        return A();
    else
        return B();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    IFoo obj = makeInterface(tid.x);
    outputBuffer[0] = genericFunc<IFoo>(obj, 3.0);
//CHECK:                               ^ cannot specialize generic with existential type
//CHECK:                               ^ specializing 'genericFunc' with an existential type is not allowed.
// CHECK: E33180
}
