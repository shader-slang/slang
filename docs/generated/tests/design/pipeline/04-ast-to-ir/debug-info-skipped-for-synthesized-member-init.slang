//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#debug-info-gating
//META: doc_section_digest=3469efef26e982a46345ae8762272897f6128f72f6bbab55e23668e85fd25748
//META: purpose=A Slang-synthesized member-wise initializer is marked `[Artificial]` on both its IR function and its `DebugFunction` record, and with debug information requested its body carries `DebugLine` instructions as user code does.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies how debug information treats a Slang-synthesized initializer.
// `Plain` has no user-written `__init`, so the member-wise initializer lowered
// for it is generated code. Lowering gives it the same debug records as a
// user-written constructor, with `DebugLine`s on the member declaration lines,
// and marks both the IR function and its `DebugFunction` record `[Artificial]`,
// so a debugger can recognize the generated code and choose its own stepping
// policy for it.
//
// The `-g2` dump embeds this file's text in a `DebugSource` string, so the
// CHECKs below anchor at the start of a line (`{{^}}`) or contain a quote,
// which the string escapes, to match the IR and not the embedded copy of
// these lines. The `CHECK-NOT` on a line that is only `}` keeps the
// `DebugLine` match inside the initializer's body.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -g2 -dump-ir -o - -stage compute -entry main

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

// CHECK: {{^}}func %main
// CHECK: {{^}}	DebugLine(
// CHECK: {{^}}[Artificial]
// CHECK-NEXT: DebugFunction("Plain.$init",
// CHECK: [nameHint("Plain.$init")]
// CHECK-NEXT: {{^}}[Artificial]
// CHECK-NEXT: {{^}}func %{{[A-Za-z0-9_]+}}
// CHECK-NOT: {{^}}}
// CHECK: {{^}}	DebugLine(
// CHECK-NOT: {{^}}}
// CHECK: {{^}}	return_val(
