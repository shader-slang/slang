//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=c20a44348c0739e8c347913c83450bf1d0dc3c34d664be2becfd728a37d9b098
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
