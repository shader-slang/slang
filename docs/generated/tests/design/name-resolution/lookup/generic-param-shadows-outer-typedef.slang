//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#generic-parameters
//META: doc_section_digest=d1f4a2e54c58257bd988b1303a72ffcd341afb497149e825b71ca8dc1d343750
//META: purpose=A generic parameter T shadows a same-named decl in an enclosing scope because the GenericDecl scope is the closer parent during the unqualified-lookup walk.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "A reference to `T` from inside the generic's inner
// decl resolves through the inner scope's parent (the GenericDecl
// scope), shadowing any same-named decl in the enclosing scope."
// Here, a file-scope `typedef float T` is shadowed by the generic
// parameter `T` of `handle`; the call with `int` selects int (would
// fail to compile if T were resolving to `float`).

//TEST:INTERPRET(filecheck=CHECK):

typedef float T;       // file-scope T = float

interface IInteger { int asInt(); }
extension int : IInteger { int asInt() { return this; } }

int handle<T : IInteger>(T x) { return x.asInt(); }

void main()
{
    // CHECK: r=7
    printf("r=%d\n", handle<int>(7));
}
