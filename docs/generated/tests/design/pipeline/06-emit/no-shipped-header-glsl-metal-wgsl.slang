//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#preludes
//META: doc_section_digest=33e1e96dddd1f86973fe4c606e101fe8d52ddd7cb099ae0023bde143a708def5
//META: purpose=Negative: GLSL, Metal, and WGSL ship no `prelude/` header, so their emitted text never includes a `slang-*-prelude.h`; Metal's only include is the platform's `metal_stdlib`, which is a system header rather than a Slang prelude.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the complement of the preludes table. Each of these three backends
// emits whatever built-in vocabulary it needs from its own emitter, so a
// `slang-*-prelude.h` include appearing here would mean a target had been
// wrongly added to the prelude registry. The CHECK-NOT patterns match the
// shared `prelude.h` file-name suffix over the whole output. The Metal prefix
// additionally asserts `metal_stdlib` is present, so the negative result is
// not just an empty compile. This file's own name deliberately avoids the
// substring, since the emitted `#line` directives quote the source path.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = int(tid.x) + 7;
}

// GLSL: {{^}}#version
// GLSL-NOT: prelude.h

// METAL: {{^}}#include <metal_stdlib>
// METAL-NOT: prelude.h

// WGSL: @compute
// WGSL-NOT: prelude.h
