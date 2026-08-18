//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -trace-coverage-binding <index> <space> pins the synthesized __slang_coverage buffer at that explicit SPIR-V Binding/DescriptorSet, and -trace-coverage-reserved-space <space> makes auto-allocation skip the reserved descriptor set and land on the next one.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::TraceCoverageBinding` and
// `OptionKind::TraceCoverageReservedSpace` arms of `OptionsParser::_parse` in
// slang-options.cpp (~53 uncovered lines between them). `TraceCoverageBinding`
// reads two unsigned operands and also implies `-trace-coverage`;
// `TraceCoverageReservedSpace` reads one and appends it to a repeatable list of
// spaces the auto-allocator must avoid.
//
// The shader declares one `RWStructuredBuffer` of its own, which takes
// `DescriptorSet 0`, so with `-trace-coverage` alone the synthesized
// `__slang_coverage` buffer auto-allocates just past it to `Binding 0,
// DescriptorSet 1` (AUTO). `-trace-coverage-binding 5 2` overrides both halves to
// `Binding 5, DescriptorSet 2` (EXPLICIT). `-trace-coverage-reserved-space 1`
// keeps auto-allocation but declares set 1 off-limits, so the buffer lands one
// set further out at `DescriptorSet 2` while its binding stays 0 (RESERVED) —
// which is the delta that distinguishes the reserved-space arm from the explicit
// one. Each is pinned from the `OpDecorate %__slang_coverage` lines, the tight
// distinguishing tokens. All three runs compile the identical shader, so the
// flags are the sole cause of the deltas.

//TEST:SIMPLE(filecheck=AUTO):-target spirv-asm -entry main -stage compute -trace-coverage
//TEST:SIMPLE(filecheck=EXPLICIT):-target spirv-asm -entry main -stage compute -trace-coverage -trace-coverage-binding 5 2
//TEST:SIMPLE(filecheck=RESERVED):-target spirv-asm -entry main -stage compute -trace-coverage -trace-coverage-reserved-space 1

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = int(tid.x);
}

//AUTO: OpDecorate %__slang_coverage Binding 0
//AUTO-NEXT: OpDecorate %__slang_coverage DescriptorSet 1

//EXPLICIT: OpDecorate %__slang_coverage Binding 5
//EXPLICIT-NEXT: OpDecorate %__slang_coverage DescriptorSet 2

//RESERVED: OpDecorate %__slang_coverage Binding 0
//RESERVED-NEXT: OpDecorate %__slang_coverage DescriptorSet 2
