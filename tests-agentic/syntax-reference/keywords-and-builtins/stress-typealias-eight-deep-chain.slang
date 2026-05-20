//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=232bb23f0daa18d59b047831901c0b444bf01283e5c09bd1d859fc8824f19b7c
//META: purpose=Stress: an eight-deep chain of `typealias` declarations must collapse end-to-end to `int` for overload resolution to select the `int` arm.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Extends `decl-typealias.slang` by chaining eight `typealias`
// declarations. Any intermediate name that failed to resolve to its
// underlying type would either fail to compile or select the wrong
// overload.

//TEST:INTERPRET(filecheck=CHECK):

typealias T1 = int;
typealias T2 = T1;
typealias T3 = T2;
typealias T4 = T3;
typealias T5 = T4;
typealias T6 = T5;
typealias T7 = T6;
typealias T8 = T7;

int kind(int v)  { return 1; }
int kind(uint v) { return 2; }

void main()
{
    T8 x = 99;
    // CHECK: k=1 v=99
    printf("k=%d v=%d\n", kind(x), int(x));
}
