//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=The diff meta-module declares `IDifferentiable`; a user struct conforming to it with `Differential = This` is accepted by the front-end without explicit import.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `diff.meta.slang` declares `IDifferentiable`. Because the diff
// meta-module is implicitly in scope, user code can declare a struct
// conforming to `IDifferentiable` without an explicit `import`. We
// observe acceptance by compiling a minimal struct that implements
// the interface methods on HLSL.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

struct MyDiff : IDifferentiable
{
    typealias Differential = MyDiff;
    float v;

    [Differentiable]
    static MyDiff dzero() { MyDiff r; r.v = 0.0; return r; }

    [Differentiable]
    static MyDiff dadd(MyDiff a, MyDiff b) { MyDiff r; r.v = a.v + b.v; return r; }

    [Differentiable]
    static MyDiff dmul(float s, MyDiff a) { MyDiff r; r.v = s * a.v; return r; }
}

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    MyDiff a; a.v = float(tid.x);
    MyDiff b; b.v = 1.0;
    MyDiff c = MyDiff.dadd(a, b);
    outBuf[tid.x] = c.v;
}

// HLSL: void main
