//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=b57e81781278bd6752bd817f50c238aae4f56d4fda646e22befd826b34d6cab4
//META: purpose=A function body parsed after the declaration pass resolves names against the scope chain that was live at its declaration site, not at file scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a deferred body is re-entered with the scope chain captured
// where the function was declared. `caller` is declared inside namespace N
// and calls `callee` unqualified, but `callee` is only declared afterwards,
// so the call can only bind once the body is parsed in the second stage —
// and it can only bind to N's member if the restored chain still starts at
// N's scope rather than wherever the deferred parse is driven from. CHECK
// asserts 42, which requires the unqualified `callee` to have reached the
// namespace member returning 41.

//TEST:INTERPRET(filecheck=CHECK):

namespace N
{
    int caller() { return callee() + 1; }
    int callee() { return 41; }
}

void main()
{
    // CHECK: r=42
    printf("r=%d\n", N::caller());
}
