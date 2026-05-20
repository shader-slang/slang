//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=monomorphization / specialization: a generic function is replaced with concrete copies per type argument set, so the emitted target code carries no generic type parameter and computes the concrete result.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "monomorphization" / "specialization" glossary
// entries: a generic identity helper instantiated at `int` is
// monomorphized away by the IR specialization pass. The
// interpreter executes the specialized form and prints the
// concrete result.

//TEST:INTERPRET(filecheck=CHECK):

T identity<T>(T v) { return v; }

void main()
{
    int a = identity<int>(11);
    // CHECK: mono=11
    printf("mono=%d\n", a);
}
