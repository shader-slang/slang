//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=f4322db59ceb8b8af42c196b566ca82e3f4804c47a61babf133a4e241eddf511
//META: purpose=`dyn` is a simple modifier on an `interface` declaration that marks the interface for dynamic dispatch; declaring a `dyn interface` with a conforming struct parses and survives into a clean HLSL / SPIR-V emit.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `dyn` in the simple-modifier table with AST node
// `DynModifier`. The user-visible probe is to declare a
// `dyn interface` with `[anyValueSize(N)]`, conform a struct, and
// have the compiler accept and emit the program. If the parser did
// not recognize `dyn`, this file would fail to compile and FileCheck
// would report no match. We pin two text-emit targets so the
// recognition is observable independent of any single backend.

//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv-asm -entry main -stage compute

[anyValueSize(16)]
dyn interface IComputeOp
{
    int compute(int x);
}

struct AddOne : IComputeOp
{
    int compute(int x) { return x + 1; }
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // Direct call through a concrete struct: avoids any need for
    // host-side type-conformance setup, which is what makes the
    // claim observable in a pure compile-only test.
    AddOne op;
    outBuf[0] = op.compute(int(41));
    //CHECK_HLSL: void main
    //CHECK_SPV: OpEntryPoint GLCompute %main "main"
}
