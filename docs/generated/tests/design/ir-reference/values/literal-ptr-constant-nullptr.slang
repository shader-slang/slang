//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5f27d4bb5fbd1429627db68bec594389689c236998e7182eaaf350ae3ceda2ee
//META: purpose=A `nullptr` literal becomes a `ptr_constant` whose inline payload the IR dump renders as the bare token `<ptr>` at each use site.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the pointer-literal case of the inline-payload spelling
// table. Like the other constants, a `ptr_constant` is folded into its
// uses instead of getting a definition line, and the printer has a
// dedicated case for it that emits `<ptr>` rather than a numeric value
// with a type clause. Comparing a null pointer against `nullptr` puts
// two of them in operand position at once, so CHECK can assert the
// exact rendered form `cmpEQ(<ptr>, <ptr>)` inside `main`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Ptr<int> p = nullptr;
    buf[0] = (p == nullptr) ? 1 : 0;
}

// IR-LABEL: func %main
// IR: : Bool	= cmpEQ(<ptr>, <ptr>)
