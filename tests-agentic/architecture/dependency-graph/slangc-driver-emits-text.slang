//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:59:05+00:00
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/architecture/dependency-graph.md#notable-invariants
//META: doc_section_digest=f07179a73dae3e0e42bafd2eeae675c59d495befe1b03e0570069dff2900a4f2
//META: purpose=`source/slang/` is the only target carrying AST/IR/emit; `slangc` links against `slang` (and `core`) and exposes the full lex-to-emit pipeline at the CLI.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the slangc-observable consequence of the `slangc -> slang`
// and `slangc -> core` edges plus the invariant "every other binary
// that needs compilation services links against `slang` rather than
// reaching into individual files": a single `slangc` invocation runs
// the full lex -> parse -> check -> IR -> emit pipeline and produces
// target text. If `slangc` did not link `slang`, the compile would
// fail at parse / check / lower / emit; the HLSL emit therefore
// carries evidence of every pipeline stage having run.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = int(tid.x);
    int y = x * 3 + 1;
    //CHECK: RWStructuredBuffer
    //CHECK: main
    outBuf[tid.x] = y;
}
