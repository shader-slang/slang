//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=533753bf58b03097d708a72394179634eea26760dccf152ebe2bcd53f0cfb0c0
//META: purpose=`extension <Type> { ... }` parses into an ExtensionDecl whose members become callable on the target type.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Add a `totalCents` method to `Wallet` via an `extension` block. The
// call `w.totalCents()` resolves only if the extension parsed and
// attached the method to the type.

//TEST:INTERPRET(filecheck=CHECK):

struct Wallet { int dollars; };

extension Wallet
{
    int totalCents() { return dollars * 100; }
};

void main()
{
    Wallet w;
    w.dollars = 5;
    // CHECK: c=500
    printf("c=%d\n", w.totalCents());
}
