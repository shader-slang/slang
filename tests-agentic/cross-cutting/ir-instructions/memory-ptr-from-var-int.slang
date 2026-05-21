//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=Boundary probe of the documented `Ptr<T>` type operand: a `rwstructuredBufferGetElementPtr` result is typed `Ptr(Int)`, exercising the doc's `Ptr` type opcode with a scalar `valueType`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `Ptr` as a type opcode (C++ wrapper `PtrType`,
// operand shape `(valueType, accessQualifier?, addressSpace?,
// dataLayout?)`). The existing `type-ptr-from-var-ir.slang` exercises
// `var` producing `Ptr<T>` for a struct; here we exercise the same
// `Ptr` opcode where the value type is a scalar `Int`, via the
// `rwstructuredBufferGetElementPtr` SSA value's documented `Ptr(Int)`
// result type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a;
}

// IR-LABEL: func %main
// IR: Ptr(Int) = rwstructuredBufferGetElementPtr(%buf, 0 : Int)
