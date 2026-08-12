//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#keepalivedecoration
//META: doc_section_digest=d34e0f2bcedd453fd31c422261c36c5f46ad56f6477c76132c40ce111506452c
//META: purpose=The front-end attaches a keepAlive decoration to a global_param so it survives DCE even with no in-IR use chain reaching it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the same DCE-suppression role on a global parameter that the
// entry point never reads: `unusedParam` has no use at all, so its survival
// in the dump is only explicable by a `keepAlive` marker. The CHECK asserts
// the decoration exists in a module whose only unreferenced declaration is
// that parameter.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;
uniform int unusedParam;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [keepAlive]
