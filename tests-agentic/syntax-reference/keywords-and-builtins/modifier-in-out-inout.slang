//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=f4322db59ceb8b8af42c196b566ca82e3f4804c47a61babf133a4e241eddf511
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
