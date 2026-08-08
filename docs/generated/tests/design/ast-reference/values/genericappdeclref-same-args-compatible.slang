//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#declref-family
//META: doc_section_digest=b67d89bff5896245b545bbc405d514054390bdb74cb2116fcc2a8cceefae2e64
//META: purpose=A generic decl-ref with its arguments applied is identified by the argument list, so two spellings of the same generic with identical arguments denote one type and values of each are mutually assignable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the positive half of generic-application identity: three
// separate `Box<int>` declarations supply identical arguments, so they
// must denote the same type. The test both initializes from and assigns
// between them, exercising the identity in both directions, and CHECK
// asserts the payload reaches all three.

//TEST:INTERPRET(filecheck=CHECK):

struct Box<T> { T v; }

void main()
{
    Box<int> a;
    a.v = 7;
    Box<int> b = a;       // same generic args -> assignable
    Box<int> c;
    c = b;                // same generic args -> assignable
    // CHECK: v=7,7,7
    printf("v=%d,%d,%d\n", a.v, b.v, c.v);
}
