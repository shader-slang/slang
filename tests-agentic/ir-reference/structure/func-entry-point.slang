//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ir-reference/structure.md#func
//META: doc_section_digest=1ffac9b7fdcad876f978c7247b8626d96221178b89de6dd5e191f70ce0142862
//META: purpose=An entry-point function lowers to a func parent opcode whose Func(...) result type carries the signature.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The compute entry point `main` should appear as `func %main : Func(Void)`
// — the `func` parent opcode with its return-and-parameter types
// recorded on the `Func(...)` result type of the opcode itself.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = 1;
}

// CHECK: func %main	: Func(Void)
