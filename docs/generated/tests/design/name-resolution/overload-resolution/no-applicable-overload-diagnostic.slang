//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=de0d5bd1ea808c72b5b22ad5fbfc389ef05a460b63cf6e8aa1c1f87a20e14622
//META: purpose=When no candidate in the overload set is Applicable for the given argument types, the resolver emits no-overload-applicable-with-args / ambiguous-call-with-args (code 39999) with a candidate note per rejected overload.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "No candidate is Applicable. The single highest-scoring
// candidate's Status says which step failed first; the resolver
// re-runs the pipeline in ForReal mode on that candidate so the
// user sees the most-specific diagnostic". Here neither overload
// accepts a struct argument so no candidate is applicable. The
// resolver also emits a per-candidate rejection note (expected vs.
// actual argument type); non-exhaustive lets the primary diagnostic
// be the asserted contract while those candidate notes vary.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Custom { int v; }

int handle(int x)   { return 1; }
int handle(float x) { return 2; }

void main()
{
    Custom c;
    int t = handle(c);
//CHECK:          ^ no overload for 'handle' applicable to arguments of type (Custom)
}
