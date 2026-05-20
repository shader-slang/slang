//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#container-level-overload-accumulation
//META: doc_section_digest=abaee536cf032e619ae16ca54b65e279f6271cecbdc254213692b2f9ca5f2f4e
//META: purpose=Same-named functions in one container accumulate into an overload set; overload resolution picks each by argument type at the call site.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The lookup doc says same-name decls accumulate into a LookupResult
// and overload resolution disambiguates by argument type. Verify
// both `twice(int)` and `twice(float)` resolve correctly.

//TEST:INTERPRET(filecheck=CHECK):

int twice(int x) { return x * 2; }
float twice(float x) { return x * 2.0; }

void main()
{
    // CHECK: i=14 f=3.5
    printf("i=%d f=%g\n", twice(7), twice(1.75f));
}
