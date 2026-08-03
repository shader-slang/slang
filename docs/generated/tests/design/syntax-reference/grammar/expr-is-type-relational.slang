//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=18781d96c73775d674cac19ebc4add31667c41ea657c3ad84686dcc6190a3a49
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
