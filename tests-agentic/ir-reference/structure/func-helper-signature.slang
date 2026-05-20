//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ir-reference/structure.md#func
//META: doc_section_digest=1ffac9b7fdcad876f978c7247b8626d96221178b89de6dd5e191f70ce0142862
//META: purpose=A user-named helper function lowers to func %helper with the signature on its Func(...) result type and parameters as Param children of the entry block.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A user-defined `int addPair(int a, int b)` should appear as
// `func %addPair : Func(Int, Int, Int)` with two `param` declarations on
// its entry block in declaration order.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

int addPair(int x, int y)
{
    return x + y;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = addPair(a, b);
}

// CHECK: func %addPair	: Func(Int, Int, Int)
// CHECK: param %x	: Int
// CHECK: param %y	: Int
