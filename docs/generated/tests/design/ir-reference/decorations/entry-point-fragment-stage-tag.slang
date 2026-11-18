//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:32+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entrypoint--entrypointdecoration
//META: doc_section_digest=f91249d2e8a42fc3ec6ad95c39ad5d65e4982723fb7beeb3095464f47cc54d67
//META: purpose=Boundary - a [shader("fragment")] entry point lowers to an entryPoint decoration whose profile integer is 5, the Stage tag for fragment (pixel) in the documented tag table.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented Stage tag table: fragment is 5 (vertex is 1 and
// compute is 6 in sibling tests), so the first operand of `entryPoint` is
// the SlangStage value and not an arbitrary constant.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage fragment

struct VOut { float4 pos : SV_Position; };

[shader("fragment")]
float4 main(VOut i) : SV_Target
{
    return i.pos;
}

// CHECK: [entryPoint(5 : Int, "main", "{{.*}}")]
// CHECK: func %main
