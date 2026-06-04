//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ir-reference/structure.md#func
//META: doc_section_digest=1ffac9b7fdcad876f978c7247b8626d96221178b89de6dd5e191f70ce0142862
//META: purpose=Function-level decorations like NameHint attach to the func opcode itself rather than to its body, per the doc's notable-opcodes note on func.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc's `### func` notable-opcode discussion, decorations
// such as `NameHintDecoration` attach to the `func` inst itself
// rather than to its body. A user-defined `pickMax` should emit
// `[nameHint("pickMax")]` immediately before the `func %pickMax`
// header line.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

int pickMax(int x, int y)
{
    return x > y ? x : y;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = pickMax(a, b);
}

// CHECK: [nameHint("pickMax")]
// CHECK-NEXT: func %pickMax	: Func(Int, Int, Int)
