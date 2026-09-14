//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f838bb38946c9317084e1b1f5f1506ec7f1d275bbff13548688e1f8693bd9850
//META: purpose=Fires diagnostic E29117 (spirv-instruction-without-result-id) - instruction has no result-id operand
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=29117
//META: catalog_name=spirv-instruction-without-result-id
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An inline `spirv_asm` instruction may only be bound to a result name when the
// opcode actually produces a result-id. `OpReturn` produces none, so binding it
// to `result:` is rejected and the CHECK pins that error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv
[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    spirv_asm
    {
        result: $$int = OpReturn
    };
}

//CHECK: E29117
