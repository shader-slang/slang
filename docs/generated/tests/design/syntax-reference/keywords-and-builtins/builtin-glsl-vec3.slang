//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=12582dfa01eb81163b717e59e208875fece8e3cb827d3738a483bbc260d0c7d8
//META: purpose=`vec3` is a GLSL-flavored name supplied by `glsl.meta.slang`; it is available when targeting GLSL emit.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Construct a `vec3` and store one of its components to a buffer.
// The compile succeeds because `glsl.meta.slang` contributes `vec3`
// to the default environment. The CHECK looks for `vec3` in the
// emitted GLSL text.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    vec3 v = vec3(1.0, 2.0, 3.0);
    //CHECK: vec3
    outBuf[0] = v.x;
}
