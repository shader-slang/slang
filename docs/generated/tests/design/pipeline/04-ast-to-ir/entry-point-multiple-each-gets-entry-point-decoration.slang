//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=d246c5b26aea56aab3063bb078408e447e5266c12518eab4e032ae8f4ee5812b
//META: purpose=When two FuncDecls are registered as entry points in one translation unit, each lowered IRFunc carries its own `entryPoint(...)` decoration; the module-level entry-point list contains both.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: module-level entry-point-count "multiple" axis. The
// doc says lowering records "the list of entry-point IR functions
// (each lowered FuncDecl that was registered as an entry point)".
// At count = 2, two distinct `entryPoint(...)` decorations appear
// on two distinct `func %name` definitions.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -entry second -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a;
}

[numthreads(1, 1, 1)]
void second(uint3 tid : SV_DispatchThreadID)
{
    buf[1] = a + 1;
}

// CHECK: [entryPoint({{.*}}, "main", {{.*}})]
// CHECK: func %main
// CHECK: [entryPoint({{.*}}, "second", {{.*}})]
// CHECK: func %second
