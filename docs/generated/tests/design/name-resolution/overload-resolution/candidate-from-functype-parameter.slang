//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=09e8a26796107f4ea870ece7161025de2c11ac3fe6f559cd6a8ba18ea88a706d
//META: purpose=A parameter whose declared type is spelled with functype takes the function-value candidate path, so an ordinary function name passed as that argument becomes the callee.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the function-valued-parameter candidate family. The parameter is
// declared with the `functype` spelling, so calling it inside the body has to
// resolve against the passed-in function value rather than a declared
// callable; CHECK asserts the sum the passed function computes.

//TEST:INTERPRET(filecheck=CHECK):

int apply(functype(int, int) -> int f, int a, int b) { return f(a, b); }

int addUp(int a, int b) { return a + b; }

void main()
{
    int tag = apply(addUp, 3, 4);
    // CHECK: tag=7
    printf("tag=%d\n", tag);
}
