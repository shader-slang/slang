//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#requirement-keys
//META: doc_section_digest=e9382240af63f219a58e7c63fe673b0ab63a2f07e00bd9fea6cb148728ad0738
//META: purpose=Each ordinary interface method requirement gets its own per-decl `IRStructKey` (`= key` in the dump) carrying a `key_`-prefixed mangled linkage name that unifies the key across modules.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ordinary-requirement shape of getInterfaceRequirementKey:
// two methods on one interface produce two distinct named `key` insts
// (per requirement decl, not shared), and a key carries a linkage
// decoration whose name starts with `key_` and embeds the mangled
// interface/method names — the cross-module unification handle.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IQuack
{
    int quack(int x);
    int waddle();
}

struct Duck : IQuack
{
    int amount;
    int quack(int x) { return amount + x; }
    int waddle() { return amount * 2; }
}

uniform int u;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Duck d;
    d.amount = u;
    buf[0] = d.quack(u) + d.waddle();
}

// Two distinct per-decl keys, one per requirement:
// CHECK-DAG: %IQuackx5Fquack : _ = key
// CHECK-DAG: %IQuackx5Fwaddle : _ = key
// Linkage name is the key_<mangled> spelling:
// CHECK-DAG: ("key_{{[^"]*}}IQuack{{[^"]*}}quack{{[^"]*}}")
