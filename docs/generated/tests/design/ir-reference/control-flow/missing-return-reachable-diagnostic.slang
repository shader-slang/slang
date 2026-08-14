//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#missingreturn-and-unreachable
//META: doc_section_digest=8dc2e6325e6131bb53f09d067b428f04953d9d1b20f2b7ffd7f4a73c37ecab2b
//META: purpose=Negative: when the block that missingReturn terminates turns out to be reachable, the later dataflow check reports it with E41010 and E41009.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the obligation that distinguishes missingReturn from unreachable:
// the terminator exists so a dataflow pass can decide whether the block is
// reachable and complain if it is. Here the helper is called from the entry
// point, so the fall-off-the-end path is live and both diagnostics fire.
// The carets sit on the function signature rather than the missing return
// statement, because that is where the reachability check reports.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

int mustReturn(int v)
/*CHECK:
    ^^^^^^^^^^ non-void function does not return in all cases
    ^^^^^^^^^^ non-void function must return
    ^^^^^^^^^^ non-void function must return in all cases for target 'spirv'
*/
{
    if (v > 0)
        return 1;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = mustReturn(n);
}
