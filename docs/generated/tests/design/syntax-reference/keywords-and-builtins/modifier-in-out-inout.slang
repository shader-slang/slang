//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=c20a44348c0739e8c347913c83450bf1d0dc3c34d664be2becfd728a37d9b098
//META: purpose=`in`, `out`, and `inout` are simple parameter-direction modifiers; an `out`/`inout` parameter mutation is visible at the caller.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `doubleIt` reads from an `in` parameter and writes to an `out`
// parameter; `swap` uses two `inout` parameters. Caller-visible
// mutation proves the direction modifiers are taking effect.

//TEST:INTERPRET(filecheck=CHECK):

void doubleIt(in int x, out int y) { y = x * 2; }
void swap(inout int a, inout int b) { int t = a; a = b; b = t; }

void main()
{
    int o = 0;
    doubleIt(7, o);
    // CHECK: o=14
    printf("o=%d\n", o);

    int a = 1;
    int b = 2;
    swap(a, b);
    // CHECK: a=2 b=1
    printf("a=%d b=%d\n", a, b);
}
