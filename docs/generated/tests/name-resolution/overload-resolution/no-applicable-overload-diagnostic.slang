//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=02bdaa234074e40f6f48635687cd0c1cf99fd86fc36701fbe4f22573b9384f99
//META: purpose=When no candidate in the overload set is Applicable for the given argument types, the resolver emits no-overload-applicable-with-args / ambiguous-call-with-args (code 39999) with a candidate note per rejected overload.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "No candidate is Applicable. The single highest-scoring
// candidate's Status says which step failed first; the resolver
// re-runs the pipeline in ForReal mode on that candidate so the
// user sees the most-specific diagnostic". Here neither overload
// accepts a struct argument so no candidate is applicable.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Custom { int v; }

int handle(int x)   { return 1; }
int handle(float x) { return 2; }

void main()
{
    Custom c;
    int t = handle(c);
//CHECK:          ^ no overload for 'handle' applicable to arguments of type (Custom)
//CHECK: E39999
}
