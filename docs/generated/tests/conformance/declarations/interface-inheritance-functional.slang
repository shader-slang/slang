//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#interfaces
//META: doc_section_digest=a18c3296228874543a504555179e0ed00ec448f24436111e6f7990ab61491195
//META: purpose=An interface may have an inheritance clause listing base interfaces; a type conforming to the derived interface must also conform to all base interfaces.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that conforming to a derived interface also satisfies its base interface.
// `Impl : IDerived` must provide both `getBase()` and `getDerived()`; the generic
// `sumInterface<T : IDerived>` calls both, and 30 proves both methods resolved.

//TEST:INTERPRET(filecheck=CHECK):

interface IBase { int getBase(); }
interface IDerived : IBase { int getDerived(); }

struct Impl : IDerived
{
    int getBase() { return 10; }
    int getDerived() { return 20; }
}

int sumInterface<T : IDerived>(T v) { return v.getBase() + v.getDerived(); }

void main()
{
    Impl obj;
    // CHECK: 30
    printf("%d\n", sumInterface(obj));
}
