//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:50:00Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#unreachable-code-diagnostics-come-from-lowering
//META: doc_section_digest=16495c9fa99361ea1a42f11df48f0494e99d8fb018fdca929c78af4b7764317f
//META: purpose=A statement placed inside a switch body before the first case label is diagnosed once, as unreachable code (E41000), on the first statement of the leading run.
//META: intent=negative
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// lowerSwitchCases warns once for the whole leading run. Two statements precede
// the first label; only the first carries the warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute

uniform int a;
RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    switch (a)
    {
        buf[1] = 5;
/*CHECK:
        ^^^ unreachable code detected
*/
        buf[2] = 6;
    case 1:
        buf[0] = 1;
        break;
    }
}
