//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=Indexed access into a local fixed-size array with a runtime index lowers to the documented `getElement` memory opcode taking `(base, index)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `getElement` as the rvalue counterpart of
// `getElementPtr`, operand shape `(base, index)`. The bootstrap
// `memory-getelement-ir.slang` exercises this on a vector; here we
// exercise it on the array shape that the doc actually mentions
// alongside `getElementPtr`, with a runtime index so the operand is
// not a literal.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int idx;
uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int arr[4] = { a, a+1, a+2, a+3 };
    buf[0] = arr[idx];
}

// IR-LABEL: func %main
// IR: getElement(%arr, %idx)
