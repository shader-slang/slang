//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=981125a6c3119290dc85c7e2ef21bc96155d4ca3760c011f47ca6e6b10fd54c9
//META: purpose=A switch with 10 cases plus default lowers to a single OpSwitch instruction with all 10 case values and a single default label.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: ten-arm OpSwitch with a default. Many compilers
// switch lowering strategies (jump table vs. binary tree) at
// some case-count threshold; the doc treats every switch as a
// single OpSwitch, so SPIR-V emit should not rewrite this.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = int(tid.x);
    int r;
    switch (v) {
        case 0:  r = 0;  break;
        case 1:  r = 1;  break;
        case 2:  r = 2;  break;
        case 3:  r = 3;  break;
        case 4:  r = 4;  break;
        case 5:  r = 5;  break;
        case 6:  r = 6;  break;
        case 7:  r = 7;  break;
        case 8:  r = 8;  break;
        case 9:  r = 9;  break;
        default: r = -1; break;
    }
    outBuf[tid.x] = r;
}

// CHECK: OpSwitch %{{[a-zA-Z_0-9]+}} %{{[a-zA-Z_0-9]+}} 0 %{{[a-zA-Z_0-9]+}} 1 %{{[a-zA-Z_0-9]+}} 2 %{{[a-zA-Z_0-9]+}} 3 %{{[a-zA-Z_0-9]+}} 4 %{{[a-zA-Z_0-9]+}} 5 %{{[a-zA-Z_0-9]+}} 6 %{{[a-zA-Z_0-9]+}} 7 %{{[a-zA-Z_0-9]+}} 8 %{{[a-zA-Z_0-9]+}} 9 %{{[a-zA-Z_0-9]+}}
