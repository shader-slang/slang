//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-c-wgsl-legalization-lowering-phi-elimination
//META: doc_section_digest=7f402a030d783be73ebd9f61d618704d309d68695b9ed3f5583606a450130887
//META: purpose=legalizeBoolSwitchForTargetsRequiringIntSwitch rewrites a bool-selector switch into an integer switch on WGSL
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the WGSL arm of legalizeBoolSwitchForTargetsRequiringIntSwitch,
// which runs immediately before legalizeIRForWGSL in Phase C. WGSL switch
// selectors must be integers, so a `switch` over a `bool` has to be rebuilt
// with an integer selector and integer case labels. CHECK asserts the
// selector expression is the bool-to-int `select` form and that the case
// labels became `i32(1)` / `i32(0)` rather than `true` / `false`.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool sel = tid.x > 0u;
    int v = 0;
    switch (sel)
    {
    case true: v = 10; break;
    case false: v = 20; break;
    }
    outBuf[tid.x] = v;
}

// CHECK: switch(select(i32(0), i32(1)
// CHECK-DAG: case i32(1)
// CHECK-DAG: case i32(0)
