//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A conformance constraint `T : IInterface` means T implements all requirements of IInterface, enabling all interface methods to be called on T inside the generic body.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: if T has a conformance constraint T : U, type T is considered
// to conform to U, meaning T implements all requirements of U.

//TEST:INTERPRET(filecheck=CHECK):

interface IStringable
{
    int code();
}

struct Alpha : IStringable
{
    int code() { return 65; }
}

struct Beta : IStringable
{
    int code() { return 66; }
}

int getCode<T : IStringable>(T x) { return x.code(); }

void main()
{
    Alpha a;
    Beta  b;
    // CHECK: 65
    printf("%d\n", getCode(a));
    // CHECK: 66
    printf("%d\n", getCode(b));
}
