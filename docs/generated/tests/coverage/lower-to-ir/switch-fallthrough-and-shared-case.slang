//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:43:23Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies adjacent empty cases share one label and a non-`break` case falls through into the following case body.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `lowerSwitchCases` / `getLabelForCase` fall-through handling. `case 0`
// is empty so it reuses `case 1`'s label (trivial fall-through). `case 2` has
// a body but no `break`, so its block falls into the `default` body: the
// emitted HLSL shows `r = 20;` immediately followed by the default body
// `r = r + 5;` inside the case-2 arm. Emitted on HLSL because the slangi VM
// aborts on switch in a function (see Unreachable gaps).
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
RWStructuredBuffer<int> outputBuffer;
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int r = 0;
    switch (int(tid.x))
    {
    case 0:
    case 1:
        r = 10;
        break;
    case 2:
        r = 20;
    default:
        r = r + 5;
        break;
    }
    outputBuffer[0] = r;
}
// CHECK: case int(0):
// CHECK-NEXT: case int(1):
// CHECK: r_{{[0-9]+}} = int(20);
// CHECK: r_{{[0-9]+}} = r_{{[0-9]+}} + int(5);
