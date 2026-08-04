//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=6dd59415374c5af65d07340c8540b315850c081483ba8c4243ef7e32cac24212
//META: purpose=A `makeTuple(...)` call lowers to the `makeTuple` IR opcode producing a `tuple_type(...)` value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Aggregate constructors table lists `makeTuple` for
// "`MakeTupleExpr`". Using the `makeTuple(...)` builtin with two
// uniform operands produces a value of `tuple_type(Int, Float)`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform float fa;
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    Tuple<int, float> t = makeTuple(a, fa);
    buf[0] = t._0;
    fbuf[0] = t._1;
}

// IR-LABEL: func %main
// IR: makeTuple(%a, %fa)
