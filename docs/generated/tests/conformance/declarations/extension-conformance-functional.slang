//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#extensions
//META: doc_section_digest=ec41e5f3bc9673746ac30b0bb1f19e54c7c9d47f0377ab73cc170b70580c168d
//META: purpose=An extension with an inheritance clause introduces a new conformance; the extended type then satisfies the interface and can be used where the interface is required.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "When an extension declaration lists an interface in its
// inheritance clause, it asserts that the extension introduces a new
// conformance."

//TEST:INTERPRET(filecheck=CHECK):

interface IPrintable { void printSelf(); }

struct MyVec { float x, y; }

extension MyVec : IPrintable
{
    void printSelf() { printf("(%g, %g)\n", x, y); }
}

void print<T : IPrintable>(T v) { v.printSelf(); }

void main()
{
    MyVec v;
    v.x = 1.0;
    v.y = 2.0;
    // CHECK: (1, 2)
    print(v);
}
