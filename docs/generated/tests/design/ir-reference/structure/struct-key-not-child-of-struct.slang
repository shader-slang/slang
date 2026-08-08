//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#key--structkey
//META: doc_section_digest=d2ea86e1e20890cc6353bda3406625017f20105e88f02a8d8e91e8431b3fecb3
//META: purpose=A struct does not own its keys: the key insts sit at module scope and the struct's field children merely point at them by operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ownership boundary the doc draws between `struct` and `key`.
// The CHECK-NOT lines require that no `= key` definition appears between the
// `struct %Vec2i` header and either of its `field` rows, and the trailing CHECK
// then finds the key definitions after the struct body has closed — i.e. at
// module scope, where code outside the struct can reference them.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int seed;
uniform RWStructuredBuffer<int> buf;

struct Vec2i
{
    int first;
    int second;
}

[numthreads(1, 1, 1)]
void main()
{
    Vec2i v;
    v.first = seed;
    v.second = seed + 1;
    buf[0] = v.first + v.second;
}

// CHECK: struct %Vec2i	: Type
// CHECK-NOT: = key
// CHECK: field(%first, Int)
// CHECK-NOT: = key
// CHECK: field(%second, Int)
// CHECK: let  %first	: _	= key
