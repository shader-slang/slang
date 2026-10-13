//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:48:26Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=7bc481f1925fc74337357f76e854855a3f8e8c8a24448fe55a29ec6c5a43f576
//META: purpose=Outside HLSL, a trailing `const` is diagnosed but remains attached to the function for recovery.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -no-codegen

struct Value
{
    int field;

    [mutating]
    void reset() const
    {
        field = 0;
    }
}

// The dialect diagnostic proves that the suffix is rejected in Slang mode. The conflict with
// `[mutating]` proves that the parser still attached the dedicated suffix modifier for semantic
// recovery instead of discarding it after reporting the dialect error.

//CHECK: E30443
//CHECK: use `[nonmutating]` or `[mutating]` on a Slang method declaration to state receiver mutability
//CHECK: E30444
