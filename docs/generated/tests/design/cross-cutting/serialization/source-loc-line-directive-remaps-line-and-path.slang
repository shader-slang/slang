//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#source-location-serialization
//META: doc_section_digest=ae999a9ca10102d601efa25676c15bb84b077c450aa17af1e8655c2470159d1f
//META: purpose=A location covered by a #line directive keeps both halves of its adjusted record, so emit renders the remapped line index and the overridden path while a location outside the directive's reach still renders the physical file.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `#line` override is represented as a physical line record plus a
// remapped line index and an overridden path, and the remapping is kept
// beside the physical record rather than folded into it. The observable
// consequence is that a single file renders both forms: the `[noinline]`
// helper sits before the directive and has no covering entry, so its
// location renders with this file's own name and its own physical line,
// while the entry point sits after the directive and renders with the
// overridden path `remapped-source.slang` and a line index in the 900s.
// Asserting both in one emit is what shows the two coexist - a table that
// had baked the remapping in would have lost the physical spelling, and
// one that had dropped the adjustment would never print the overridden
// path. The helper is `[noinline]` so it survives to emit as its own
// function and keeps a location of its own.
//
// The claim is about the retained locations rather than about one back
// end, so both halves are asserted on every text target that can render a
// quoted path. GLSL and WGSL default to a rendering that carries a file
// number instead of a path, so they are asked for `-line-directive-mode
// standard`; on WGSL that mode is the only way the retained locations
// surface at all, since the target emits no directives by default. The
// two CHECK-DAG lines per target form one unordered group because the
// targets do not agree on which of the two they emit first. SPIR-V is
// recorded as unsupported-on-target in the bundle README: it has no
// `#line` concept, and even under `-g2` its debug info names only the
// physical file, never the overridden path.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> outBuf;

[noinline]
int physicalHelper(int v)
{
    return v * 3;
}

#line 900 "remapped-source.slang"

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = physicalHelper(int(tid.x)) + 7;
}

// The physical half: this file's own path, from a location the directive
// does not cover.
// HLSL-DAG: #line {{[0-9]+}} "{{.*}}source-loc-line-directive-remaps-line-and-path.slang"
// The adjusted half: the overridden path plus a line index in the 900s.
// HLSL-DAG: #line 90{{[0-9]}} "remapped-source.slang"

// GLSL-DAG: #line {{[0-9]+}} "{{.*}}source-loc-line-directive-remaps-line-and-path.slang"
// GLSL-DAG: #line 90{{[0-9]}} "remapped-source.slang"

// METAL-DAG: #line {{[0-9]+}} "{{.*}}source-loc-line-directive-remaps-line-and-path.slang"
// METAL-DAG: #line 90{{[0-9]}} "remapped-source.slang"

// WGSL-DAG: #line {{[0-9]+}} "{{.*}}source-loc-line-directive-remaps-line-and-path.slang"
// WGSL-DAG: #line 90{{[0-9]}} "remapped-source.slang"

// CUDA-DAG: #line {{[0-9]+}} "{{.*}}source-loc-line-directive-remaps-line-and-path.slang"
// CUDA-DAG: #line 90{{[0-9]}} "remapped-source.slang"

// CPP-DAG: #line {{[0-9]+}} "{{.*}}source-loc-line-directive-remaps-line-and-path.slang"
// CPP-DAG: #line 90{{[0-9]}} "remapped-source.slang"
