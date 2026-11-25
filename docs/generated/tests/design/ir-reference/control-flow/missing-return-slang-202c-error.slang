//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:00Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#missingreturn-and-unreachable
//META: doc_section_digest=26a97d045a79c541f13053bb869f73d7230d355079d7c493c6899dba3041d07a
//META: purpose=Negative: for Slang 202c and later a reachable fall-off-the-end path in a value-returning function is reported as error 40025 (missing-return-not-allowed-in-slang-202c) at the function signature.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists three diagnostics reported by checkForMissingReturns at the
// missingReturn's source location. With -std 202c the 40025 error applies
// instead of 41010 / 41009. The helper is called from the entry point so the
// fall-off path is reachable.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 202c -target spirv -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

int mustReturn(int v)
/*CHECK:
    ^^^^^^^^^^ non-void function must return in Slang 202c and later
    ^^^^^^^^^^ non-void function must return in all cases in Slang 202c and later
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
