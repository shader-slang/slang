//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=d6a8e6a85f98f1fe1e7c4215cf8b2b2f8cf50505ce91badfb5ce987b8c7527a4
//META: purpose=The synthesized ThisTypeDecl member of an InterfaceDecl makes `This` resolve inside the interface's own requirement signatures.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `This` resolves inside an interface body as the abstract
// self type: the interface owns a synthesized member decl for it, so the
// name is reachable from the interface's own scope even though no parser
// scope is ever pushed for it. `scaled` is declared to return `This`, and
// a generic constrained to the interface chains `scaled(2).area()`; that
// chain only type-checks if `This` resolved to the conforming type, so the
// printed area of the scaled square proves the resolution.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape
{
    This scaled(int k);
    int area();
}

struct Square : IShape
{
    int side;
    This scaled(int k) { Square r; r.side = side * k; return r; }
    int area() { return side * side; }
}

int areaOfScaled<T : IShape>(T v, int k)
{
    return v.scaled(k).area();
}

void main()
{
    Square q;
    q.side = 3;
    // CHECK: a=36
    printf("a=%d\n", areaOfScaled(q, 2));
}
