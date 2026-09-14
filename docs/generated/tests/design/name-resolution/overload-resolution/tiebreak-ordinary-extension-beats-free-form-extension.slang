//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=02381f6488db1154095a241fb46e92defb884509c6c7ee2e2bc10be1811be455
//META: purpose=CompareLookupResultItems prefers an ordinary extension over a free-form extension whose target type is a constrained generic type parameter.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the free-form-extension half of the lookup-result comparison. Both
// extensions contribute an equally applicable member of the same name to the
// candidate set, and the free-form one is spelled by extending a constrained
// generic type parameter; CHECK asserts the ordinary extension's tag.

//TEST:INTERPRET(filecheck=CHECK):

interface IFoo { int f(); }

struct S : IFoo { int f() { return 1; } }

extension S { int g() { return 1; } }

extension<T : IFoo> T { int g() { return 2; } }

void main()
{
    S s;
    int tag = s.g();
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
