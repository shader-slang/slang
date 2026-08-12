//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#property
//META: doc_section_digest=2be36ef488309471e001ccf502f4e364705629af21bac8d6137274483b7c4b14
//META: purpose=A `property` with both `get` and `set` accessors: reading the property calls `get`; assigning to it calls `set` with the new value as `newValue`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that property reads dispatch to `get` and writes dispatch to `set` with `newValue`.
// The set body updates both m_val and m_valIsPositive; buffer output confirms the side effects
// and the get-returned value are consistent across positive and negative assignments.
// Emission tests confirm the property get accessor and struct are emitted on all backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct TestClass
{
    float m_val;
    bool m_valIsPositive;

    property someProp : float
    {
        get
        {
            return m_val;
        }
        set
        {
            m_val = newValue;
            m_valIsPositive = (newValue > 0.0f);
        }
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    TestClass obj = { };
    obj.someProp = 3.0f;
    outputBuffer[0] = (int)obj.m_val;
    outputBuffer[1] = (int)obj.m_valIsPositive;
    obj.someProp = -1.0f;
    outputBuffer[2] = (int)obj.m_valIsPositive;
    float got = obj.someProp;
    outputBuffer[3] = (int)got;
}

// CHECK: 3
// CHECK: 1
// CHECK: 0
// CHECK: -1

// HLSL: struct TestClass_{{[0-9_]*}}
// HLSL: float m_val_{{[0-9_]*}}
// HLSL: TestClass_someProp_get_{{[0-9_]*}}

// GLSL: struct TestClass_{{[0-9_]*}}
// GLSL: float m_val_{{[0-9_]*}}
// GLSL: TestClass_someProp_get_{{[0-9_]*}}

// METAL: struct TestClass_{{[0-9_]*}}
// METAL: float m_val_{{[0-9_]*}}
// METAL: TestClass_someProp_get_{{[0-9_]*}}

// WGSL: struct TestClass_{{[0-9_]*}}
// WGSL: m_val_{{[0-9_]*}}
// WGSL: fn TestClass_someProp_get_{{[0-9_]*}}

// CUDA: struct TestClass_{{[0-9_]*}}
// CUDA: TestClass_someProp_get_{{[0-9_]*}}

// CPP: struct TestClass_{{[0-9_]*}}
// CPP: TestClass_someProp_get_{{[0-9_]*}}
