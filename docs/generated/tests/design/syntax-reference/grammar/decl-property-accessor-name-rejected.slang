//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=d2933f044671826cdda47d2bf6584eb3ef9b48c4c8f5e77a17b011c61c8729d4
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
