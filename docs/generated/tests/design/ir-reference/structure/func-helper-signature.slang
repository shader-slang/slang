//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#func
//META: doc_section_digest=deb5e1ca3ce3504c71e6588591557fba17b76b32217464f0d32bb1a25b6f910c
//META: purpose=A user-named helper function lowers to func %helper with the signature on its Func(...) result type and parameters as Param children of the entry block.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two halves of the `func` shape at once: the declared signature
// is the inst's own `Func(returnType, paramTypes...)` result type, and the
// declared parameters are `param` children of the entry block (not operands of
// `func`). CHECK pins `func %addPair : Func(Int, Int, Int)` and then the two
// `param` lines that follow inside the body.

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
