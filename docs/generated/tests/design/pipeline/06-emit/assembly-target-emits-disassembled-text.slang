//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#emit-dispatcher
//META: doc_section_digest=f136369c763a469783c526b150d8baf5d8f6435a816fbf58504eaa5c871b9d36
//META: purpose=An `*Assembly` target such as `spirv-asm` is served by compiling the corresponding binary target and then disassembling it, so the output is disassembler text rather than emitter-authored source.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the assembly-target arm of the outer dispatcher. If `-target
// spirv-asm` were a source emitter of its own, the text would be whatever the
// emitter chose to print; because it is a recursive compile of `-target spirv`
// followed by disassembly, the output opens with the disassembler's banner
// comments (`; SPIR-V`, `; Version:`, `; Generator:`, `; Bound:`) which no
// Slang source emitter writes. CHECK asserts that banner precedes the module's
// first real instruction.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) * 3;
}

// SPIRV: {{^}}; SPIR-V
// SPIRV: {{^}}; Version:
// SPIRV: {{^}}; Generator:
// SPIRV: {{^}}; Bound:
// SPIRV: OpCapability Shader
