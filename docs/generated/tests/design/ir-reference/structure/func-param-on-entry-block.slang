//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#func
//META: doc_section_digest=8bdd4611fe4bde5f67b7b4377a4dac327c9defdb39a0f44cd421464470e7c1fd
//META: purpose=The entry block of a func owns its function parameters as param children in declaration order.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a function's parameters are block parameters of its entry
// block, in declaration order. A three-parameter helper is compiled and the
// ordered CHECK lines (which advance monotonically through the dump) require
// `param %a`, `param %b`, `param %c` to appear in that sequence after the
// `func %sumTrio` label; any reordering or hoisting of the params onto the
// `func` itself would break the sequence.

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
