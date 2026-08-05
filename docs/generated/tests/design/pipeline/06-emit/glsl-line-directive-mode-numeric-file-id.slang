//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#emit-dispatcher
//META: doc_section_digest=f136369c763a469783c526b150d8baf5d8f6435a816fbf58504eaa5c871b9d36
//META: purpose=GLSL targets resolve to the GLSL line-directive mode, so their `#line` directives carry a numeric source-string number instead of the quoted file name the C-style mode uses.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-target line-directive-mode resolution the dispatcher does
// before constructing the SourceWriter. GLSL has no file-name form of `#line`,
// so a C-style directive would be a syntax error for glslang. CHECK asserts
// the GLSL emit contains `#line <n> <n>` and, via CHECK-NOT, that no quoted
// file name appears in a directive; the HLSL prefix asserts the C-style
// quoted form on the same source so the two modes are contrasted directly.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);

    outBuf[0] = a * 2;
}

// GLSL: {{^}}#line {{[0-9]+}} {{[0-9]+$}}
// GLSL-NOT: {{^}}#line {{[0-9]+}} "
// HLSL: {{^}}#line {{[0-9]+}} "{{.*}}.slang"
