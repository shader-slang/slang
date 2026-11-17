//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entrypoint--entrypointdecoration
//META: doc_section_digest=f91249d2e8a42fc3ec6ad95c39ad5d65e4982723fb7beeb3095464f47cc54d67
//META: purpose=A function marked with shader/numthreads attributes lowers to a func with an entryPoint decoration carrying profile-int, entry-name, and module-name operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the three-operand shape of `entryPoint`: an integer profile tag,
// the user-visible entry name as a string, and the module name. The CHECK
// pins the operand *kinds* (`{{[0-9]+}} : Int`, then `"main"`) and
// wildcards the module name, which is derived from the source file name and
// would otherwise make the test filename-dependent.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(8, 4, 2)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [entryPoint({{[0-9]+}} : Int, "main", "{{.*}}")]
// CHECK: func %main
