//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#subscript-op
//META: doc_section_digest=624917b929b5db01e3231965412253236cf0ba05012e0fb465fd200622c5ec90
//META: purpose=A `__subscript` operator with `get` and `set` accessors lets the struct be accessed with `[]`; reading invokes `get`, writing invokes `set`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `__subscript` operator routes reads to `get` and writes to `set`.
// Three elements are written via x[i] = v and read back via float v = x[i]; buffer output
// confirms each element retains the assigned value. Emission tests confirm the struct and
// array field are emitted on all backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct Indexed
{
    float arr[4];

    __subscript(int i) -> float
    {
        get { return arr[i]; }
        set { arr[i] = newValue; }
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Indexed x = { };
    x[0] = 10.0f;
    x[1] = 20.0f;
    x[2] = 30.0f;

    float v0 = x[0];
    float v1 = x[1];
    float v2 = x[2];
    outputBuffer[0] = (int)v0;
    outputBuffer[1] = (int)v1;
    outputBuffer[2] = (int)v2;
}

// CHECK: 10
// CHECK: 20
// CHECK: 30

// HLSL: struct Indexed_{{[0-9_]*}}
// HLSL: float arr_{{[0-9_]*}}

// GLSL: struct Indexed_{{[0-9_]*}}
// GLSL: float arr_{{[0-9_]*}}

// METAL: struct Indexed_{{[0-9_]*}}
// METAL: arr_{{[0-9_]*}}

// WGSL: struct Indexed_{{[0-9_]*}}
// WGSL: arr_{{[0-9_]*}}

// CUDA: struct Indexed_{{[0-9_]*}}
// CUDA: arr_{{[0-9_]*}}

// CPP: struct Indexed_{{[0-9_]*}}
// CPP: arr_{{[0-9_]*}}
