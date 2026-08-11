//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=667410b964ed99fbf0c70015e2ab77e7656e9f8728c17d66aa0a1cad3ef6f6b5
//META: purpose=`is` is an infix operator at relational level 6 whose right operand is a Type, binding tighter than `==` (level 7).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both halves of the `is` claim: its right operand parses as
// a Type (a struct name), and it sits at level 6, so
// `s is Circle == false` groups as `(s is Circle) == false` — the
// alternative grouping `s is (Circle == false)` is not even a type.
// Both dynamic outcomes are checked through an interface-typed value.

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

int classify(IShape s)
{
    if (s is Circle == false)
    {
        return 100;
    }
    return 200;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Circle c;
    Square q;
    // CHECK: 200
    outputBuffer[0] = classify(c);
    // CHECK: 100
    outputBuffer[1] = classify(q);
}
