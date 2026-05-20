//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/wgsl.md#specializeaddressspaceforwgsl
//META: doc_section_digest=55422c2a92758737eb6eb0ac0f489a44b06df5c80b1e2907ff718d43d17483b7
//META: purpose=Module-scope static variables become var<private> with the initializer fired inside main
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A module-scope 'static int g = 7' becomes a var<private> WGSL
// global; its initializer is moved into the entry-point body by
// moveGlobalVarInitializationToEntryPoints.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

static int g_counter = 7;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = g_counter;
}

// CHECK: var<private>
// CHECK-SAME: g_counter_{{[0-9]+}}
// CHECK: fn main
// CHECK: g_counter_{{[0-9]+}} = i32(7)
