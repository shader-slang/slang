//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entrypoint--entrypointdecoration
//META: doc_section_digest=f91249d2e8a42fc3ec6ad95c39ad5d65e4982723fb7beeb3095464f47cc54d67
//META: purpose=A non-entry helper called from main does not carry entryPoint, while main does, so the decoration is scoped to the entry-point function alone.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `entryPoint` is what a link-time pass can use to select the
// functions exposed in the final binary: it must appear on exactly the
// entry point. The module has one entry point and one ordinary helper; the
// CHECK-DAG lines assert the `entryPoint` decoration names `"main"` while
// the helper appears only through its `nameHint`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

int helperOnly(int x) { return x + 17; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helperOnly(int(tid.x));
}

// CHECK-DAG: [entryPoint({{[0-9]+}} : Int, "main", "{{.*}}")]
// CHECK-DAG: [nameHint("helperOnly")]
