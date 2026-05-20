//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T10:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=9ce4d053661212c26b75e2e5610820fbcc296703c400512972d2afca16087b34
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
