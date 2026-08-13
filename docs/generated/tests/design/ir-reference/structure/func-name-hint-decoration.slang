//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#func
//META: doc_section_digest=deb5e1ca3ce3504c71e6588591557fba17b76b32217464f0d32bb1a25b6f910c
//META: purpose=Function-level decorations like NameHint attach to the func opcode itself rather than to its body.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a function-level decoration is printed on the `func` inst and
// not on any inst inside its body. CHECK matches `[nameHint("pickMax")]` and
// then requires the *immediately following* line (CHECK-NEXT) to be the
// `func %pickMax` header, which is only true if the decoration is attached to
// the `func` rather than to a block or a body instruction.

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
