//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizenonvectorcompositeselect
//META: doc_section_digest=91945b91f0ef5f8c97fb42298e0ccc4af95a00d2644244d36b722f26f75038ac
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
