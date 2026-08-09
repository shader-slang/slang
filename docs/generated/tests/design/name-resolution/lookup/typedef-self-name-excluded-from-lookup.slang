//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#concepts
//META: doc_section_digest=82fc9ca4bda6256917442becba961ab7af693eb5db7c9a9e9bf691de4244f8b8
//META: purpose=The declaration being checked is excluded from its own lookup, so a typedef that reuses the name it is defined from resolves that name to the outer declaration.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the self-exclusion the doc illustrates with `typedef Foo
// Foo;`. Inside namespace N the typedef and its right-hand side share
// the spelling Foo; without excluding the decl under check, the alias
// would resolve to itself. CHECK asserts 21 read back through a field
// of the file-scope struct, which is only reachable if Foo inside N
// denotes that struct.

//TEST:INTERPRET(filecheck=CHECK):

struct Foo { int v; }

namespace N
{
    typedef Foo Foo;

    int use() { Foo f; f.v = 21; return f.v; }
}

void main()
{
    // CHECK: r=21
    printf("r=%d\n", N::use());
}
