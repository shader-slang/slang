//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary
//META: doc_section_digest=a84f2b8e553f246fd0e2611d7bd357de1ceb312fa259d862c4056388b6d6a68a
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
