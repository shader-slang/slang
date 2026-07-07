//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=d86bedaed855595e10c45be817dcd07ae903a5905ada08a17d56c55e59daa232
//META: purpose=A function with an `out T` parameter has IR type `OutParam(T)`; the callee writes through the parameter with `store`, and the caller passes a `Ptr(T)` from a local `var`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Conversions table lists `outImplicitCast` for "Function-arg
// lowering for `out` parameters", and `inOutImplicitCast` for
// `inout`. When the caller's argument type matches the parameter
// type the cast is not emitted, but the parameter's IR type is
// still `OutParam(<T>)`; this test anchors on that shape.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

void writeOut(out int o)
{
    o = 42;
}

[numthreads(1, 1, 1)]
void main()
{
    int x;
    writeOut(x);
    buf[0] = x;
}

// IR: param %{{.*}}: OutParam(Int)
// IR-LABEL: func %main
// IR: var
// IR: call %writeOut(%{{.*}})
