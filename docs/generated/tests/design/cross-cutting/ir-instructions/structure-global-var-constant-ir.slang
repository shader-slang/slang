//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=d6d21d15e8b14ba2b57495447b3188f156d4c4168b8574097962151b591b1497
//META: purpose=A mutable module-scope `static` lowers to `global_var` while a `static const` lowers to `globalConstant`, both at module scope alongside `global_param`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `global_var, global_param, globalConstant` structure row and
// its `Global` flag note: these opcodes always live at module scope. One
// translation unit declares all three surfaces — a mutable `static int`, a
// `static const int`, and a `uniform` shader parameter. The CHECKs assert
// each prints outside any `func` body (they appear before the `func %main`
// label in the dump) with the opcode the row names, and that the
// `globalConstant` carries its initializer as an operand rather than needing
// a store, which is the difference between it and `global_var`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

static int counter = 0;
static const int LIMIT = 7;

uniform int a;
uniform RWStructuredBuffer<int> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    counter = counter + a;
    outbuf[0] = counter + LIMIT;
}

// IR-DAG: global_var %counter : Ptr(Int)
// IR-DAG: let %a : Int = global_param
// IR-DAG: let %LIMIT : Int = globalConstant(7 : Int)
// IR: func %main
