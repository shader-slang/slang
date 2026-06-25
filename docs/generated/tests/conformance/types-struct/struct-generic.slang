//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#syntax
//META: doc_section_digest=362ffce5f577a08a16fa66e283951f5a11bdf34ab8552fa1aa839abf43be9765
//META: purpose=A struct may have generic type parameters; instantiating the generic at different type arguments creates distinct struct types.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The struct-decl grammar includes [generic-params-decl], meaning structs can be generic.
// A generic struct with one type parameter T wrapping a value of type T. A helper function
// is used to prevent the struct from being constant-folded away at the call site. Buffer
// output confirms the value flows through; emission tests confirm the specialized struct
// appears on all text-emit backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct Box<T>
{
    T value;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

Box<int> makeBox(int v)
{
    Box<int> b;
    b.value = v;
    return b;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Non-constant input prevents constant folding
    Box<int> ibox = makeBox((int)tid.x + 42);
    outputBuffer[0] = ibox.value;
}

// CHECK: 42

// Generic struct specialized to Box<int> survives on all targets.
// HLSL: struct Box_{{[0-9_]*}}
// HLSL: int value_{{[0-9_]*}}

// GLSL: struct Box_{{[0-9_]*}}
// GLSL: int value_{{[0-9_]*}}

// METAL: struct Box_{{[0-9_]*}}
// METAL: int value_{{[0-9_]*}}

// WGSL: struct Box_{{[0-9_]*}}
// WGSL: value_{{[0-9_]*}}

// CUDA: struct Box_{{[0-9_]*}}
// CUDA: int value_{{[0-9_]*}}

// CPP: struct Box_{{[0-9_]*}}
// CPP: value_{{[0-9_]*}}
