//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#per-global-parameter-steps
//META: doc_section_digest=71acd681692af8dc698e27f4202784a0fdd9ad10068cb56ebbb1ea587605026a
//META: purpose=The per-global-parameter loop runs once per field of the global struct layout, so two distinct global parameters produce two distinct structFieldLayout entries.
//META: intent=boundary
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the loop nature of the per-global-parameter step by moving from
// one global to two. CHECK matches two separate `structFieldLayout(%bufX`
// entries, so the layout module recorded a field per global rather than
// collapsing them; a single-global test cannot distinguish "loop" from
// "one-shot".

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> bufA;
uniform RWStructuredBuffer<float> bufB;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bufA[0] = int(tid.x);
    bufB[0] = float(tid.y);
}

// CHECK-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// CHECK: structFieldLayout(%buf{{[A-Z]}}
// CHECK: structFieldLayout(%buf{{[A-Z]}}
