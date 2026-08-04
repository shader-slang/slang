//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T16:30:00Z
//META: source_commit=110ca68915f1b6690e19a647e3437e1ad1f78a03
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#builtin-operators-bypass-call-lowering
//META: doc_section_digest=e70a29d1d218a8caa31a9b05ae27e6e9e79c2c46f8f65751d8428df1ad6141c2
//META: purpose=A builtin `a % b` on integer operands lowers to an `irem` IR opcode — the doc says `%` picks `IRIRem` for non-floating-point operands.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the element-type selection for `%`: non-floating-point
// operands pick the integer-remainder opcode. The dump opcode is
// `irem` (integer remainder) rather than `mod` — the IR-side
// spelling of `IRIRem`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a % b;
}

// CHECK-LABEL: func %main
// CHECK: irem(%{{.*}}, %{{.*}})
