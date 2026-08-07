//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=d2d46e039ebd707b280ca397a6adb57c2b8df7a613cf8bf7da56564fc99b5920
//META: purpose=A FuncDecl registered as an entry point ends up in the lowered module with an `entryPoint(...)` decoration on its IRFunc.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Module-level outputs" section records "The list of
// entry-point IR functions (each lowered FuncDecl that was registered
// as an entry point)." Observable via the `entryPoint(...)`
// decoration emitted just above the `func %main` line in the IR dump.
// This anchors on the AST-side claim ("lowered FuncDecl registered
// as entry point") rather than the per-decoration catalog (covered
// in `cross-cutting/ir-instructions`).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = 1;
}

// CHECK: [entryPoint({{.*}}"main"{{.*}})]
// CHECK-NEXT: [numThreads
// CHECK: func %main
