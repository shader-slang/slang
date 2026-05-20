//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:41Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/ast-reference/index.md#cross-cutting-topics
//META: doc_section_digest=36f1c960f3b34c6c84e64ac6cb1dc61adf1ed16c98013e2b8374b4a2928c2950
//META: purpose=The index doc's cross-cutting topics name parse, semantic-check, and AST-to-IR lowering as the AST's life cycle; a struct declaration survives all three phases and appears as a struct in emitted HLSL target text.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The #cross-cutting-topics section cites pipeline/02-parse-ast,
// pipeline/03-semantic-check, and pipeline/04-ast-to-ir as the
// AST's life cycle. The end-to-end claim is that a source-level
// declaration travels through all three and reaches the target
// emitter. A user-named struct field is the simplest cross-cutting
// evidence: it must survive the parser (StructDecl + VarDecl + Type),
// the checker (QualType resolution), and lowering (IR struct type),
// and finally appear in the emitted HLSL text.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Payload
{
    int value;
    float weight;
}
// CHECK: struct Payload
// CHECK: int value
// CHECK: float weight

RWStructuredBuffer<Payload> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Payload p;
    p.value = int(tid.x);
    p.weight = float(tid.x) * 0.5;
    outBuf[0] = p;
}
