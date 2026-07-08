//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#types
//META: doc_section_digest=24919d68577e8ac7fef57e804fdf1d69f9063278aeb46ec9c6ca7e3bf3539893
//META: purpose=`CoreType` accepts the function-type spelling `'functype' '(' Type (',' Type)* ')' '->' Type`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `functype` keyword spelling of a function type: a
// typealias names `functype (int, int) -> int`, a named function is
// bound to a variable of that type, and the value is invoked. The
// emitted C++ containing the entry point proves the whole chain
// (including the functype parse) compiled.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -stage compute

typealias BinOp = functype (int, int) -> int;

int add(int a, int b)
{
    return a + b;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    BinOp f = add;
    // CHECK: computeMain
    outputBuffer[0] = f(40, 2);
}
