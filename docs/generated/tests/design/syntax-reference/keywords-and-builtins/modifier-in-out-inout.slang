//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=14131d1346ae80bf12a86c5f0464133e39600a2d8baef546279830e042b6cb1c
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
