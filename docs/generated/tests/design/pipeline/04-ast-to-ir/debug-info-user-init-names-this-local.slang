//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#debug-info-gating
//META: doc_section_digest=62ea8db2784ef3877410b92997d96e0414adfc1ab018c13cd6fdcaff541c61de
//META: purpose=A user-written `__init` keeps its debug information, and constructor lowering names the by-value object under construction `this` and gives it a debug-location decoration.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the other side of the constructor debug gate: `WithInit.__init` is
// user-authored, so it keeps a debug-function record and debug lines even
// though it mangles the same way a synthesized initializer would — the
// discrimination is by constructor flavor, not by name. The CHECKs also pin the
// naming rule: the local the initializer returns is given the name `this` and a
// debug-location decoration, which is what lets a debugger stopped inside the
// initializer inspect the members being written.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -g2 -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

struct WithInit
{
    int x;
    __init(int v) { x = v; }
}

[numthreads(1, 1, 1)]
void main()
{
    WithInit w = WithInit(a);
    buf[0] = w.x;
}

// CHECK: DebugFunction("WithInit.$init"
// CHECK: [nameHint("WithInit.$init")]
// CHECK: [DebugLocation(
// CHECK-NEXT: [nameHint("this")]
// CHECK: [DebugLocation(
// CHECK-NEXT: [nameHint("this")]
// CHECK-NEXT: let {{.*}} : Ptr(%WithInit) = var
// CHECK: DebugLine(
