//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#property
//META: doc_section_digest=2be36ef488309471e001ccf502f4e364705629af21bac8d6137274483b7c4b14
//META: purpose=A property that provides only a `get` accessor is read-only; assigning to it is a compile error.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a property with only a `get` accessor emits and is readable.
// ReadOnlyProp.xVal returns m_x; CHECK confirms the field name appears in the HLSL output,
// proving the get accessor wired through correctly.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

struct ReadOnlyProp
{
    int m_x;

    property xVal : int
    {
        get { return m_x; }
    }
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    ReadOnlyProp obj = { 77 };
    output[0] = obj.xVal;
}

// CHECK: m_x

// GLSL emits struct ReadOnlyProp with field and get accessor function.
// GLSL: struct ReadOnlyProp_{{[0-9]+}}
// GLSL: m_x_{{[0-9]+}}
// GLSL: ReadOnlyProp_xVal_get_{{[0-9]+}}

// Metal emits struct ReadOnlyProp with field and get accessor function.
// METAL: struct ReadOnlyProp_{{[0-9]+}}
// METAL: m_x_{{[0-9]+}}
// METAL: ReadOnlyProp_xVal_get_{{[0-9]+}}

// WGSL emits struct ReadOnlyProp with field and get accessor function.
// WGSL: struct ReadOnlyProp_{{[0-9]+}}
// WGSL: m_x_{{[0-9]+}}
// WGSL: ReadOnlyProp_xVal_get_{{[0-9]+}}

// CUDA emits struct ReadOnlyProp with field and get accessor function.
// CUDA: struct ReadOnlyProp_{{[0-9]+}}
// CUDA: m_x_{{[0-9]+}}
// CUDA: ReadOnlyProp_xVal_get_{{[0-9]+}}

// CPP emits struct ReadOnlyProp with field and get accessor function.
// CPP: struct ReadOnlyProp_{{[0-9]+}}
// CPP: m_x_{{[0-9]+}}
// CPP: ReadOnlyProp_xVal_get_{{[0-9]+}}
