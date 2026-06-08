//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=f4322db59ceb8b8af42c196b566ca82e3f4804c47a61babf133a4e241eddf511
//META: purpose=`groupshared` is a simple HLSL-flavored storage modifier; it lowers to `groupshared` in HLSL emit and to `shared` in GLSL emit.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per `_common.md`'s multi-backend rule, this modifier's spelling
// differs per target, so we test the emit of every text-emit backend
// where the difference is observable. HLSL keeps the original
// keyword; GLSL maps it to `shared`.

//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry main -stage compute

groupshared int sharedBuf[4];

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    sharedBuf[tid.x] = int(tid.x);
    //CHECK_HLSL: groupshared
    //CHECK_GLSL: shared
}
