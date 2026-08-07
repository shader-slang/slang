//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=078844f9de8080a010d3c70d4af44f40667b8ce158b13e5adda01e318abce99c
//META: purpose=A failed generic-application speculation leaks no diagnostic and does not move the real token reader, so `a < b > c` parses as two chained comparisons.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two guarantees the speculative disambiguator gives: the trial
// parse runs on a parser copy with a throwaway sink, and the real reader never
// advances. `a < b > c` is the shape that forces a full trial parse of `<b>` as
// a generic argument list; the token after the matching `>` is an identifier,
// which is not a generic-follower, so the trial is abandoned. The test asserts
// the compile is clean (INTERPRET fails outright on any error, so a leaked
// diagnostic from the trial sink would show up here) and that the value is the
// one chained comparison produces: `(1 < 2)` is true, and `true > 3` converts
// to `1 > 3`, which is false.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 1;
    int b = 2;
    int c = 3;
    bool r = a < b > c;
    // CHECK: r=0
    printf("r=%d\n", r ? 1 : 0);
}
