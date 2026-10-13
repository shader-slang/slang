//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=46b0c5c1edca66a7398c5ce31b5fa5af46daab931e0874ea601514ec997eb09b
//META: purpose=Fires diagnostic E33075 (cannot-have-writable-receiver-in-dyn-interface) - dyn interfaces cannot have methods with writable receivers
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33075
//META: catalog_name=cannot-have-writable-receiver-in-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL's implicit writable method receiver is rejected by a `dyn` interface.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl docs/generated/tests/design/cross-cutting/diagnostics-catalog/33075-cannot-have-writable-receiver-in-dyn-interface.slang -std 2026 -no-codegen

dyn interface IFoo
{
    void mutate(int value);
//CHECK: ^^^^^^ E33075
//CHECK: ^^^^^^ `dyn` interface member 'mutate' has a writable receiver; use `[nonmutating]` to make it read-only.
}
