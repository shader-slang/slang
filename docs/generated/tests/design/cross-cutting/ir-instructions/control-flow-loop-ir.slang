//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=0b68b77b6156c67db4c587635b263df5f1c7f5c3d702914334c92fc6b5e88c90
//META: purpose=A `for` loop lowers to the `loop` terminator and uses block `param`s to carry the SSA induction value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies two control-flow claims at once: the `loop` terminator (in
// the `TerminatorInst` family) is produced for a `for` loop, and the
// loop header carries `param` declarations ("Block or function
// parameter; replaces SSA phi").

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += i;
    buf[0] = sum;
}

// IR-LABEL: func %main
// IR: loop(%{{.*}}, %{{.*}}, %{{.*}}{{.*}})
// IR: param %{{.*}}
