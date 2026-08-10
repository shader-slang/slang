//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=0b34b0fb1813131149d3616a45cc3577cb15e3fd3da49c3edccfff26407d2c0a
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
