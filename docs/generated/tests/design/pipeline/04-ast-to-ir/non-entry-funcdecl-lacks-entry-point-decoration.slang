//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=9ce4d053661212c26b75e2e5610820fbcc296703c400512972d2afca16087b34
//META: purpose=Only FuncDecls registered as entry points carry an `entryPoint(...)` decoration; a sibling non-entry helper function in the same translation unit is lowered without that decoration.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: entry-point-registration axis — the negative direction.
// The doc's "list of entry-point IR functions (each lowered FuncDecl
// that was registered as an entry point)" implies that non-entry
// FuncDecls are lowered without the `entryPoint` decoration. Pattern
// verifies the decoration appears immediately above `func %main` but
// not above `func %helper`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int v) { return v + 7; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helper(a);
}

// CHECK-NOT: [entryPoint({{.*}})]
// CHECK-LABEL: func %helper
// CHECK: [entryPoint({{.*}}"main"{{.*}})]
// CHECK-LABEL: func %main
