//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=4e9de34fbc8a8877c6b73c6110db4d1ebef2d0ddd10da2bdfd2742413c1d6015
//META: purpose=`as` is an infix operator at relational level 6 whose right operand is parsed as a Type (AsTypeExpr).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `as` operator with a Type right operand: downcasting an
// interface-typed value yields a value whose presence tracks the
// dynamic type (present for a Circle, absent for a Square), which
// requires the parser to have built an AsTypeExpr with `Circle` read
// as a type, not an expression.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer

interface IShape
{
    int id();
}

struct Circle : IShape
{
    int id() { return 1; }
}

struct Square : IShape
{
    int id() { return 2; }
}

int probe(IShape s)
{
    let c = s as Circle;
    if (c.hasValue)
    {
        return c.value.id();
    }
    return 0;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Circle c;
    Square q;
    // CHECK: 1
    outputBuffer[0] = probe(c);
    // CHECK: 0
    outputBuffer[1] = probe(q);
}
