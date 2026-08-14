//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=6e6bd1f54cf8c6c1c7ea9efd59c3c27ea43dc0195aebd524dc7f1c903ca95457
//META: purpose=matchArgumentsToParams lets a brace initializer argument be matched against a struct parameter instead of being rejected outright.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the argument/parameter matching that runs ahead of the per-argument
// coercion check: `take({1, 2})` pairs the brace initializer with the `P`
// parameter rather than treating it as an unmatched argument. CHECK asserts the
// callee saw both field values, which proves the match and the coercion both
// succeeded.

//TEST:INTERPRET(filecheck=CHECK):

struct P { int a; int b; }

int take(P p) { return p.a * 10 + p.b; }

void main()
{
    int v = take({1, 2});
    // CHECK: v=12
    printf("v=%d\n", v);
}
