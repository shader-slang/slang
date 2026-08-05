//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies that `try f(x)` on a `throws` function emits a tagged-result check so the caller can branch to its catch handler.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `try` wraps a fallible call and turns its result into a tagged value that
// the caller tests before using. The emitted HLSL is the observation point:
// the CHECKs assert a result-carrying struct is produced by the callee and
// that the caller branches on its tag field, which is exactly what the wrapper
// has to introduce. Names are matched loosely because locals are suffixed
// during emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

int mayThrow(int x) throws int
{
    if (x < 0) throw 1;
    return x * 2;
}

int caller(int x)
{
    do { return try mayThrow(x); }
    catch (int e) { return -e; }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = caller(4 + int(tid.x));
    // HLSL: ResultType{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}} = mayThrow{{[A-Za-z0-9_]*}}(
    // HLSL: if({{[A-Za-z0-9_]+}}.tag{{[A-Za-z0-9_]*}})
}
