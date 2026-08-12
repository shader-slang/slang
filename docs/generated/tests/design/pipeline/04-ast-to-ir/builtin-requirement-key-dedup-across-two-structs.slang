//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#requirement-keys
//META: doc_section_digest=e9382240af63f219a58e7c63fe673b0ab63a2f07e00bd9fea6cb148728ad0738
//META: purpose=Two structs conforming to IDifferentiable share one hoistable `builtinRequirementKey` inst per built-in requirement — both witness tables carry an entry keyed by the same key inst.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the hoistable-key dedup the doc commits to: a recognized
// built-in requirement's key is identified by its BuiltinRequirementKind
// operand, so a second conforming decl reuses the same inst instead of
// minting a new key. The first builtinRequirementKey in the dump is
// captured and must appear as the lookup key of two distinct
// witness_table_entry rows (one in each struct's table).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct GradA : IDifferentiable { float v; }
struct GradB : IDifferentiable { float w; }

uniform float u;
uniform RWStructuredBuffer<float> buf;

float pass<T : IDifferentiable>(T t, float x) { return x; }

[numthreads(1, 1, 1)]
void main()
{
    GradA a;
    a.v = u;
    GradB b;
    b.w = u;
    buf[0] = pass<GradA>(a, u) + pass<GradB>(b, u);
}

// CHECK: [[KEY:%[0-9A-Za-z_]+]] : _ = builtinRequirementKey(
// CHECK-DAG: witness_table_entry([[KEY]]
// CHECK-DAG: witness_table_entry([[KEY]]
