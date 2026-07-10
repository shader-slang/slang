//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T16:30:00Z
//META: source_commit=110ca68915f1b6690e19a647e3437e1ad1f78a03
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=d6ee07799b141a6850f1700c9e4ae870d18815ea897d847d8fe60117b962be60
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
