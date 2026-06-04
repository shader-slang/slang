//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#property
//META: doc_section_digest=2be36ef488309471e001ccf502f4e364705629af21bac8d6137274483b7c4b14
//META: purpose=The `set` accessor may name its parameter explicitly; the named parameter is used in the set body instead of the implicit `newValue`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `set` accessor with an explicitly named parameter compiles and functions correctly.
// ExplicitSet.someProp uses `set(float newVal)`; assigning 5.0 then reading back via `get` lets
// buffer output confirm the named parameter received and stored the value. Emission tests confirm
// the get accessor and struct are emitted on all backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct ExplicitSet
{
    float m_val;

    property someProp : float
    {
        get() { return m_val; }
        set(float newVal) { m_val = newVal; }
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    ExplicitSet obj = { };
    obj.someProp = 5.0f;
    float v = obj.someProp;
    outputBuffer[0] = (int)v;
}

// CHECK: 5

// HLSL: struct ExplicitSet_{{[0-9_]*}}
// HLSL: float m_val_{{[0-9_]*}}
// HLSL: ExplicitSet_someProp_get_{{[0-9_]*}}

// GLSL: struct ExplicitSet_{{[0-9_]*}}
// GLSL: float m_val_{{[0-9_]*}}
// GLSL: ExplicitSet_someProp_get_{{[0-9_]*}}

// METAL: struct ExplicitSet_{{[0-9_]*}}
// METAL: float m_val_{{[0-9_]*}}
// METAL: ExplicitSet_someProp_get_{{[0-9_]*}}

// WGSL: struct ExplicitSet_{{[0-9_]*}}
// WGSL: m_val_{{[0-9_]*}}
// WGSL: fn ExplicitSet_someProp_get_{{[0-9_]*}}

// CUDA: struct ExplicitSet_{{[0-9_]*}}
// CUDA: ExplicitSet_someProp_get_{{[0-9_]*}}

// CPP: struct ExplicitSet_{{[0-9_]*}}
// CPP: ExplicitSet_someProp_get_{{[0-9_]*}}
