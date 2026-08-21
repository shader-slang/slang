//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#interlocked-is-a-wrapper-not-an-opcode
//META: doc_section_digest=7b87b736cd493e5a9697e064429849fbc9b5f1e3a536c25c80750e511548e528
//META: purpose=HLSL's `InterlockedAdd(dest, value, original)` is a `[ForceInline]` core-module function, so immediately after AST lowering `func %main` holds a `call`, not an `atomicAdd` opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `Interlocked*` family is a wrapper rather than an
// opcode. The call is made on an element of an `RWStructuredBuffer`, so
// the buffer element pointer and the `out` parameter's `var` are both
// visible; the CHECK matches the `call specialize(...)` that receives
// them and uses `IR-NEXT` to show that the instruction following the
// call is ordinary integer arithmetic — no atomic opcode is produced at
// this stage.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev;
    InterlockedAdd(rwbuf[idx], idx + 1, prev);
    rwbuf[idx + 1] = prev;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%rwbuf, %idx)
// IR: call specialize({{.*}})({{.*}}%prev)
// IR-NEXT: add(%idx, 1 : Int)
