//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=d6a8e6a85f98f1fe1e7c4215cf8b2b2f8cf50505ce91badfb5ce987b8c7527a4
//META: purpose=An accessor body resolves its own setter parameter from the AccessorDecl's scope and the enclosing struct's field through the PropertyDecl and struct scopes above it.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the accessor/property links in the scope chain. The setter names
// its parameter `newValue`, which must come from the accessor's own scope,
// and both accessors read the struct field `storage`, which is only
// reachable by walking out through the property to the struct's scope.
// CHECK asserts the getter doubles the stored 4 and that the setter halves
// the assigned 10 back into `storage`, so both names must have bound.

//TEST:INTERPRET(filecheck=CHECK):

struct Box
{
    int storage;

    property doubled : int
    {
        get { return storage * 2; }
        set(newValue) { storage = newValue / 2; }
    }
}

void main()
{
    Box b;
    b.storage = 4;
    // CHECK: get=8
    printf("get=%d\n", b.doubled);
    b.doubled = 10;
    // CHECK: set=5
    printf("set=%d\n", b.storage);
}
