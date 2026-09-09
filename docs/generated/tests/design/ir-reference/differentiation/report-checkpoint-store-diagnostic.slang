//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#checkpointobj-and-reportcheckpointstore
//META: doc_section_digest=89f9c7c3922f2d0de6bb09ee14de2d13e4bd96d2436ef34586e8b757c7e9a742
//META: purpose=ReportCheckpointStore is the diagnostic channel for the checkpointing machinery, and its stored type and originating function are what the checkpoint-report walk prints.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one consequence of the checkpointing markers that is reachable
// from the command line. The marker itself is inserted by the unzip pass and
// so never appears in a lower-stage dump, but the report it feeds is user
// visible under `-report-checkpoint-intermediates`. `loopy` accumulates across
// a loop, so reverse mode has to keep per-iteration primal values, forcing a
// checkpoint store. The first CHECK pins the report naming the originating
// function `loopy` — the marker's `originalFunc` operand — and the second pins
// the entry describing the stored value, which is the `storedType` operand
// rendered as a type and a byte count. Both together show the marker survived
// DCE and was consumed by the reporting walk rather than dropped. The report is
// produced by the same walk regardless of back-end, so one directive per
// text-emit target shares a single CHECK prefix.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -report-checkpoint-intermediates
//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute -report-checkpoint-intermediates
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -report-checkpoint-intermediates
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute -report-checkpoint-intermediates
//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute -report-checkpoint-intermediates
//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute -report-checkpoint-intermediates
//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute -report-checkpoint-intermediates

uniform float a;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float loopy(float x)
{
    float acc = x;
    for (int i = 0; i < 4; ++i)
        acc = acc * x + acc;
    return acc;
}

[numthreads(1, 1, 1)]
void main()
{
    var dpx = diffPair(a, 0.0);
    bwd_diff(loopy)(dpx, 1.0);
    buf[0] = dpx.d;
}

// CHECK: checkpointing context of {{[0-9]+}} bytes associated with: 'loopy'
// CHECK: {{[0-9]+}} bytes (FixedArray<float, {{[0-9]+}}> )
