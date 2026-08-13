//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=a505032fe1ced813d290018b087d7ba67b9a672a9b25244d2cf6a5fa887fe795
//META: purpose=`AccessorName` is limited to `'get' | 'set' | 'ref'`; any other accessor name is diagnosed.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an accessor block rejects a name outside the
// documented get/set/ref set. `modify` is used as the probe (it is
// not in the current AccessorName production), and the parser must
// emit its unexpected-token diagnostic E20003 on it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S
{
    int v;

    property val : int
    {
        modify { return v; }
//CHECK:^^^^^^ error E20003
//CHECK:^^^^^^ unexpected identifier
    }
}

void main()
{
}
