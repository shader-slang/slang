//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A SwitchStmt with five explicit cases lowers to a single `switch(...)` terminator carrying five case-value / case-block operand pairs in source order.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: switch-case-count axis — many cases. Five case-value/
// case-block pairs in declaration order appear inside the `switch`
// operand list (after scrutinee, merge, default). The doc's row
// describes this structure once; verifies it scales.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int sel;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    switch (sel)
    {
        case 0: buf[0] = 0; break;
        case 1: buf[0] = 1; break;
        case 2: buf[0] = 2; break;
        case 3: buf[0] = 3; break;
        case 4: buf[0] = 4; break;
        default: buf[0] = -1; break;
    }
}

// CHECK-LABEL: func %main
// CHECK: switch(%{{.*}}, %{{.*}}, %{{.*}}, 0 : Int, %{{.*}}, 1 : Int, %{{.*}}, 2 : Int, %{{.*}}, 3 : Int, %{{.*}}, 4 : Int, %{{.*}})
