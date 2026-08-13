//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#lowerdefer
//META: doc_section_digest=7ae5fe7dc9992f36f284f4dc1fd1b7a0f8a84959d24476b29a32da98a11c7a5e
//META: purpose=Phase B's lowerDefer (B3) duplicates the deferred body to the end of every dominated block whose terminator leaves the deferred region, so an early return also runs it.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the multi-exit boundary of the defer lowering. The enclosing
// scope has two exits -- an early `return` inside the `if`, and the normal
// fall-through -- so the duplication rule must place the deferred store on
// both. The condition is fed from a `uniform` so neither branch is folded
// away before the snapshot, and the CHECKs require the deferred store of
// `200` to appear twice while the `defer` opcode is gone.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    defer { buf[1] = 200; }
    if (a > 0) { buf[0] = 1; return; }
    buf[0] = 2;
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-LABEL: func %main
// CHECK: store({{.*}}, 200 : Int)
// CHECK: store({{.*}}, 200 : Int)
// CHECK-NOT: = defer(
