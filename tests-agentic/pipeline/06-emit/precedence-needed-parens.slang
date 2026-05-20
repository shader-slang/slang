//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#operator-precedence-and-parenthesization
//META: doc_section_digest=71c6e6a3dd9f98cd8871ef646710647c71e8dc0f8d4f04eda428d745f76e66a0
//META: purpose=The precedence helper keeps parentheses that are required to preserve semantics: an addition inside a multiplication context emits as `(a + b) * c` on every C-like text target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `(a + b) * c` — parens needed because `+` has lower precedence
// than `*`. The precedence helper (slang-emit-precedence.{h,cpp})
// must keep the parentheses on every C-like target so the emitted
// expression preserves the semantic grouping. Operands are derived
// from `tid` components so that the values reach emit through a
// per-target lowering that does not collapse them to literals (the
// CUDA backend in particular factors `__ldg` reads out of uniforms,
// which would split the binary expression across temporaries).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);
    int b = int(tid.y);
    int c = int(tid.z);
    outBuf[tid.x] = (a + b) * c;
}

// HLSL: ({{.*}} + {{.*}}) * {{.*}}
// GLSL: ({{.*}} + {{.*}}) * {{.*}}
// METAL: ({{.*}} + {{.*}}) * {{.*}}
// WGSL: ({{.*}} + {{.*}}) * {{.*}}
// CUDA: ({{.*}} + {{.*}}) * {{.*}}
// CPP: ({{.*}} + {{.*}}) * {{.*}}
