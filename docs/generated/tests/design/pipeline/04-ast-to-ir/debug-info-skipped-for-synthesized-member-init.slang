//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#debug-info-gating
//META: doc_section_digest=62ea8db2784ef3877410b92997d96e0414adfc1ab018c13cd6fdcaff541c61de
//META: purpose=A Slang-synthesized member-wise initializer gets no debug-line instructions or debug-location decoration, even when debug information is requested for the module.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that debug information is gated per construct, not per module.
// `Plain` has no user-written `__init`, so the initializer lowered for it has no
// user-authored body; stepping into it would walk the struct's member
// declaration lines. The first CHECK proves debug info really is on for this
// compile (user code in `main` gets `DebugLine`s); the CHECK-NOTs then require
// the synthesized `Plain.$init` body, up to its terminator, to contain neither a
// `DebugLine` nor a debug-location decoration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -g2 -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

struct Plain
{
    int x;
    int y;
}

[numthreads(1, 1, 1)]
void main()
{
    Plain p;
    p.x = a;
    p.y = a;
    buf[0] = p.x + p.y;
}

// CHECK: func %main
// CHECK: DebugLine(
// CHECK: [nameHint("Plain.$init")]
// CHECK-NOT: DebugLine(
// CHECK-NOT: DebugLocation(
// CHECK: return_val(
