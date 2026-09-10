//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#debug-levels-and--debug-info-include-source
//META: doc_section_digest=a465c2bb21f120c8c3458404a567b476edc5f751bce03ff4c08b17bf8f2b2163
//META: purpose=-debug-info-include-source at -g1 selects the File+Source form of OpSource that embeds the shader text, instead of the bare language/version form.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the orthogonal source-embedding switch. At exactly the Minimal
// debug level the option makes the emitter walk every debug source with
// content and emit one File+Source OpSource per file, and the two OpSource
// forms are mutually exclusive so the bare record must not also appear. CHECK
// asserts the OpSource line names a file id and is followed by the embedded
// text of this very shader.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -g1 -debug-info-include-source

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) - 4;
}

// CHECK: OpSource Slang 1 %{{[a-zA-Z_0-9]+}} "
