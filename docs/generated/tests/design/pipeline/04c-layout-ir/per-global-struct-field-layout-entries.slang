//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#per-global-parameter-steps
//META: doc_section_digest=71acd681692af8dc698e27f4202784a0fdd9ad10068cb56ebbb1ea587605026a
//META: purpose=Recording each stub global in the global type-layout builder yields one structFieldLayout entry naming that stub variable and its lowered varLayout.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fourth per-global-parameter step: each stub global is fed
// into the module-level struct-type-layout builder. CHECK asserts a
// `structFieldLayout(%buf, %N)` entry exists, which names the stub global
// itself as the field key and its lowered varLayout as the value - the
// pairing that step produces.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: structFieldLayout(%buf, %{{[0-9]+}})
