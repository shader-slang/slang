//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ir-reference/structure.md#func
//META: doc_section_digest=1ffac9b7fdcad876f978c7247b8626d96221178b89de6dd5e191f70ce0142862
//META: purpose=The entry block of a func owns its function parameters as param children in declaration order.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc, "the first block's Param children are the function
// parameters in declaration order". A three-parameter helper should
// emit `param %a`, `param %b`, `param %c` in that order on its entry
// block, matching the source declaration order.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int x;
uniform int y;
uniform int z;
uniform RWStructuredBuffer<int> buf;

int sumTrio(int a, int b, int c)
{
    return a + b + c;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = sumTrio(x, y, z);
}

// CHECK-LABEL: func %sumTrio	: Func(Int, Int, Int, Int)
// CHECK: param %a	: Int
// CHECK: param %b	: Int
// CHECK: param %c	: Int
