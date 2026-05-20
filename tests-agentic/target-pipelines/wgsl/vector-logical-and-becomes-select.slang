//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/wgsl.md#legalizelogicalandor
//META: doc_section_digest=77623d96e7c34fe5ce0ff46d2e739f512c1a986984dfae15e8ac26f0397159dc
//META: purpose=Logical and-or over vector operands is rewritten to a WGSL select() call by legalizeLogicalAndOr
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL forbids '&&' on vector operands, so legalizeLogicalAndOr
// rewrites the expression to select(false_value, true_value, cond).

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<bool> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool3 a = bool3(true, false, true);
    bool3 b = bool3(false, true, true);
    bool3 r = a && b;
    outBuf[tid.x] = r.x;
}

// CHECK: select(vec3<bool>
