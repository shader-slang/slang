//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T10:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A function with five-deep nested IfStmts lowers each level to its own `ifElse(...)` terminator; the body grows into many basic blocks (then / else / merge per level).
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: control-flow depth axis. With five nested `if` levels the
// IR contains five distinct `ifElse(...)` terminators in sequence —
// one per nesting level — and the function body decomposes into
// many basic blocks. Pattern verifies five ifElse occurrences
// inside `%main`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int r = 0;
    if (a > 0) {
        if (a > 1) {
            if (a > 2) {
                if (a > 3) {
                    if (a > 4) {
                        r = 5;
                    }
                }
            }
        }
    }
    buf[0] = r;
}

// CHECK-LABEL: func %main
// CHECK: ifElse(
// CHECK: ifElse(
// CHECK: ifElse(
// CHECK: ifElse(
// CHECK: ifElse(
