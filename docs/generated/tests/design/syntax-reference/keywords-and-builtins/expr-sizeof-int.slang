//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=6c39a8a833fc084f0b1f7e30400e6efc997d8e919471186717dfce4ca0bf8e11
//META: purpose=`sizeof` is an expression keyword that returns the size, in bytes, of its type operand at compile time.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `sizeof(int)` should resolve to 4 in the Slang core-module type
// system. The constructor-style cast `int(sizeof(int))` avoids the
// C-style cast that the pilot bundle reported as unreliable in the
// interpreter.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: s=4
    printf("s=%d\n", int(sizeof(int)));
}
