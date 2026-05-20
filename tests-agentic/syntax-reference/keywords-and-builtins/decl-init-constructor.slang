//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=232bb23f0daa18d59b047831901c0b444bf01283e5c09bd1d859fc8824f19b7c
//META: purpose=`__init` is the decl keyword for a constructor; it is invoked when the type name is used as a call expression.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Declare a struct with an `__init(int)` constructor and call it via
// the type-name-as-callable form `V(7)`. The doc notes that `__init`
// is the underscore-prefixed compiler-internal spelling of `init`.

//TEST:INTERPRET(filecheck=CHECK):

struct V
{
    int x;
    __init(int v) { x = v; }
}

void main()
{
    V v = V(7);
    // CHECK: x=7
    printf("x=%d\n", v.x);
}
