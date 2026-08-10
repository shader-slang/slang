//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=0b68b77b6156c67db4c587635b263df5f1c7f5c3d702914334c92fc6b5e88c90
//META: purpose=A `printf` call lowers to the variadic `Printf` opcode carrying the format string as its first operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "other control-flow / backend-hint opcodes" row, which lists
// `Printf` as variadic with a leading `format` operand. The IR CHECK asserts
// the dump prints `Printf("v=%d\n", %...)` — the format string inline as an
// operand, followed by the runtime argument — rather than a `call` to a
// core-module function. The argument comes from a `uniform` so it stays a
// non-constant operand. The CPP and CUDA CHECKs confirm the opcode reaches
// emission as a real `printf` on the host-like targets.

//TEST:SIMPLE(filecheck=IR):-target cpp -dump-ir -o /dev/null -stage compute -entry main
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    printf("v=%d\n", a);
    outbuf[0] = a;
}

// IR-LABEL: func %main
// IR: Printf("v=%d\n", %{{.*}})

// CPP: printf("v=%d\n",
// CUDA: printf("v=%d\n",
