//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=0b68b77b6156c67db4c587635b263df5f1c7f5c3d702914334c92fc6b5e88c90
//META: purpose=A `switch` statement lowers to the `switch` terminator.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `switch` terminator row. A multi-case `switch` on a
// non-constant `uniform` selector lowers to the `switch` terminator in
// the IR dump; the runtime selector keeps the branch from being folded
// to a single case.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int sel;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    switch (sel)
    {
    case 0: buf[0] = 10; break;
    case 1: buf[0] = 20; break;
    default: buf[0] = 30; break;
    }
}

// IR-LABEL: func %main
// IR: switch(%{{.*}}
