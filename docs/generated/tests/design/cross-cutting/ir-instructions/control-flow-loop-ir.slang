//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=c0238650e6ba499ae56294af00d41ede4ea9a7b4f8215393080c0974a1553619
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
