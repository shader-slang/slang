//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#legalizenonvectorcompositeselect
//META: doc_section_digest=100b87dba42c123fd6aa8b9b03923517959ac72e54b0d687a54ff8edc9f4c719
//META: purpose=HLSL-only legalizeNonVectorCompositeSelect rewrites a struct-typed conditional into an explicit if/else with a temporary.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// DXC's select() intrinsic accepts only vector conditions, so a
// `cond ? structA : structB` is rewritten by Slang into an
// explicit if/else block that writes to a named temporary.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Data { float a; float b; };
RWStructuredBuffer<Data> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Data d1 = { 1.0, 2.0 };
    Data d2 = { 3.0, 4.0 };
    bool cond = tid.x > 0;
    Data result = cond ? d1 : d2;
    outBuf[tid.x] = result;
}

// CHECK: if({{.*}})
// CHECK: else
