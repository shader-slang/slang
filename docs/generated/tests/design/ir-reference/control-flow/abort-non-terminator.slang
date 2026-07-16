//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:07:37Z
//META: source_commit=c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#abort
//META: doc_section_digest=c7693fdd37023bb5a9e34f71071374da718341c95afdd7a9ab459dbdb06785c9
//META: purpose=The abort(format, args...) builtin lowers to an Abort instruction that is not an IR terminator: like discard, it sits inside the block and the block's real terminator (unconditionalBranch to the post-if merge) follows it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies Abort is not a TerminatorInst: an abort() call placed as
// the last statement of an if-arm appears in the IR dump as an Abort
// instruction with the arm block's real terminator — the branch to
// the post-if merge — on the line after it, the same in-block shape
// the catalog documents for discard.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int g;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    if (g > 7)
        abort("g out of range");
    buf[0] = g;
}

// CHECK-LABEL: func %main
// CHECK: Abort(
// CHECK-NEXT: unconditionalBranch(%{{[0-9]+}})
