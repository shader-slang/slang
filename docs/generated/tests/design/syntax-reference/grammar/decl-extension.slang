//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=b092f350bdbde298b8860c3da710d6672d2e5f235d75620d903bddf8a6158c3b
//META: purpose=`ExtensionDecl` parses as `'extension' Type '{' StructMember* '}'` and adds methods to an existing type.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ExtensionDecl production: `extension Type { ... }` adds
// methods to an existing type. We extend a `struct Box` with a method
// and then call it from `main`.

//TEST:INTERPRET(filecheck=CHECK):

struct Box { int v; }

extension Box
{
    int doubled() { return v * 2; }
}

void main()
{
    Box b;
    b.v = 21;
    // CHECK: doubled=42
    printf("doubled=%d\n", b.doubled());
}
