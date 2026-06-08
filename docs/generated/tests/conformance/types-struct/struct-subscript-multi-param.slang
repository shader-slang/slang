//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#subscript-op
//META: doc_section_digest=624917b929b5db01e3231965412253236cf0ba05012e0fb465fd200622c5ec90
//META: purpose=Multiple `__subscript` overloads with different parameter counts are all valid; overload resolution dispatches each call to the right subscript.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that multiple `__subscript` overloads with different parameter counts coexist and
// that overload resolution dispatches each call correctly. m[] exercises the zero-parameter
// overload and m[1] the one-parameter overload; buffer output confirms both read and write paths.
// Emission tests confirm the struct and overloads are emitted on all backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct MultiSubscript
{
    float arr[4];

    // 0-parameter subscript
    __subscript() -> float
    {
        get { return arr[0]; }
        set { arr[0] = newValue; }
    }

    // 1-parameter subscript
    __subscript(int i) -> float
    {
        get { return arr[i]; }
        set { arr[i] = newValue; }
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    MultiSubscript m = { };

    m[] = 100.0f;
    m[1] = 200.0f;

    float r0 = m[];
    float r1 = m[1];

    outputBuffer[0] = (int)r0;
    outputBuffer[1] = (int)r1;
}

// CHECK: 100
// CHECK: 200

// HLSL: struct MultiSubscript_{{[0-9_]*}}
// HLSL: float arr_{{[0-9_]*}}

// GLSL: struct MultiSubscript_{{[0-9_]*}}
// GLSL: float arr_{{[0-9_]*}}

// METAL: struct MultiSubscript_{{[0-9_]*}}
// METAL: arr_{{[0-9_]*}}

// WGSL: struct MultiSubscript_{{[0-9_]*}}
// WGSL: arr_{{[0-9_]*}}

// CUDA: struct MultiSubscript_{{[0-9_]*}}
// CUDA: arr_{{[0-9_]*}}

// CPP: struct MultiSubscript_{{[0-9_]*}}
// CPP: arr_{{[0-9_]*}}
