//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=44ec80292e79604a0622888241e4998ddb07e1c6a708c0c3c0b8cfa44ba7028e
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
