//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#hlsl-semantics-sv
//META: doc_section_digest=c0d08afc8ccd44fdbb2133509498b8edd2e9e1df38204624893050548106177b
//META: purpose=`: register(...)` binding semantic propagates to emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLRegisterSemantic` is the AST node for `: register(...)`. A
// resource with `: register(u3)` must emit with that register binding
// in the resulting HLSL.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf : register(u3);

[numthreads(1,1,1)]
void main()
{
    outBuf[0] = 1;
}

//CHECK: register(u3)
