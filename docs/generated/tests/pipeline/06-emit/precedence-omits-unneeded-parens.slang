//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#operator-precedence-and-parenthesization
//META: doc_section_digest=71c6e6a3dd9f98cd8871ef646710647c71e8dc0f8d4f04eda428d745f76e66a0
//META: purpose=The precedence helper emits the minimum parentheses needed. `a * b + c` keeps neither operator wrapped (no `(a * b) + c` and no `a * (b + c)`).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `a * b + c` — `*` already binds tighter than `+`, so the
// precedence helper (slang-emit-precedence.{h,cpp}) should not
// surround the multiplication with parens. To assert this
// negatively, we route the value through a side-effectful buffer
// write and use `// CHECK-NOT:` on each target to forbid the
// `(a * b) + c` shape (a `*` followed immediately by `) +`).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);
    int b = int(tid.y);
    int c = int(tid.z);
    outBuf[tid.x] = a * b + c;
}

// HLSL: outBuf
// HLSL-NOT: {{[a-zA-Z0-9_]+}}) * {{[a-zA-Z0-9_]+}}) + {{[a-zA-Z0-9_]+}}
// GLSL: outBuf
// GLSL-NOT: {{[a-zA-Z0-9_]+}}) * {{[a-zA-Z0-9_]+}}) + {{[a-zA-Z0-9_]+}}
// METAL: outBuf
// METAL-NOT: {{[a-zA-Z0-9_]+}}) * {{[a-zA-Z0-9_]+}}) + {{[a-zA-Z0-9_]+}}
// WGSL: outBuf
// WGSL-NOT: {{[a-zA-Z0-9_]+}}) * {{[a-zA-Z0-9_]+}}) + {{[a-zA-Z0-9_]+}}
