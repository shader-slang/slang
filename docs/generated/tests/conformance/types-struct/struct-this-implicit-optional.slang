//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#nonstatic-member-function
//META: doc_section_digest=c90f11708cb380fb674a4708aaf5743e5a1f31c69b6cf4d7b4a599ffc35791bb
//META: purpose=Inside a non-static member function, `this.member` and bare `member` both refer to the same object field; the `this.` prefix is optional.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `this.member` and bare `member` are equivalent inside a non-static method.
// sumExplicit() uses `this.x + this.y`; sumImplicit() uses `x + y`; both are called on the
// same instance. Buffer output confirms both return the same sum. Emission tests confirm
// both member functions survive on all text-emit backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct Vec2
{
    int x;
    int y;

    int sumExplicit()
    {
        return this.x + this.y;
    }

    int sumImplicit()
    {
        return x + y;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Vec2 v;
    v.x = 3;
    v.y = 4;
    int r1 = v.sumExplicit();
    int r2 = v.sumImplicit();
    outputBuffer[0] = r1;
    outputBuffer[1] = r2;
}

// CHECK: 7
// CHECK: 7

// HLSL: struct Vec2_{{[0-9_]*}}
// HLSL-DAG: int Vec2_sumExplicit_{{[0-9_]*}}
// HLSL-DAG: int Vec2_sumImplicit_{{[0-9_]*}}

// GLSL: struct Vec2_{{[0-9_]*}}
// GLSL-DAG: int Vec2_sumExplicit_{{[0-9_]*}}
// GLSL-DAG: int Vec2_sumImplicit_{{[0-9_]*}}

// METAL: struct Vec2_{{[0-9_]*}}
// METAL-DAG: Vec2_sumExplicit_{{[0-9_]*}}
// METAL-DAG: Vec2_sumImplicit_{{[0-9_]*}}

// WGSL: struct Vec2_{{[0-9_]*}}
// WGSL-DAG: fn Vec2_sumExplicit_{{[0-9_]*}}
// WGSL-DAG: fn Vec2_sumImplicit_{{[0-9_]*}}

// CUDA: struct Vec2_{{[0-9_]*}}
// CUDA-DAG: Vec2_sumExplicit_{{[0-9_]*}}
// CUDA-DAG: Vec2_sumImplicit_{{[0-9_]*}}

// CPP: struct Vec2_{{[0-9_]*}}
// CPP-DAG: Vec2_sumExplicit_{{[0-9_]*}}
// CPP-DAG: Vec2_sumImplicit_{{[0-9_]*}}
