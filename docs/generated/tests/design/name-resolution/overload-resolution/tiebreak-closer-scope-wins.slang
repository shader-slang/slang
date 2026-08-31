//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=02381f6488db1154095a241fb46e92defb884509c6c7ee2e2bc10be1811be455
//META: purpose=getScopeRank prefers the declaration at the shorter scope distance from the call site when both non-generic candidates are otherwise equal.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the scope-distance step, which only runs for non-generic flavors.
// The namespace-local `pick` and the file-scope `pick` accept the call with the
// same cost and specificity; the call sits inside the namespace, so the
// namespace-local one is closer. CHECK asserts its tag.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x) { return 1; }

namespace Near
{
    int pick(int x) { return 2; }

    int call() { return pick(7); }
}

void main()
{
    int tag = Near::call();
    // CHECK: tag=2
    printf("tag=%d\n", tag);
}
