//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#per-global-parameter-steps
//META: doc_section_digest=71acd681692af8dc698e27f4202784a0fdd9ad10068cb56ebbb1ea587605026a
//META: purpose=Each per-global-parameter step attaches an IRLayoutDecoration to the stub IRGlobalVar, observable as a [layout(%N)] on the global_param in the first post-link IR snapshot.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the layout-IR module attaches a layout decoration to the
// stub global variable it materialises for every global shader parameter.
// The dump is taken at the first post-link snapshot, where the layout
// module has already been merged in; CHECK asserts the `buf` global_param
// carries a [layout(%N)] decoration, which is the only place that
// decoration can come from.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: [layout(%{{[0-9]+}})]
// CHECK: global_param
