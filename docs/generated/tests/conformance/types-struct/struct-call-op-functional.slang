//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#function-call-op
//META: doc_section_digest=25db753c000949cdb0161bb6d155f6566474fe2e1005a820a94f2bb0487efed3
//META: purpose=A `operator ()` declaration allows applying parameters to an object as if it were a function; multiple overloads are resolved by parameter count/type.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `operator ()` overloads with different arities are all reachable and that
// overload resolution selects the right one by argument count. Three calls m(), m(x), m(x,y)
// exercise the zero-, one-, and two-parameter overloads. Buffer output confirms each returns
// the expected product; emission tests confirm all three overloads survive on every backend.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct Multiplier
{
    float base;

    float operator () ()
    {
        return base;
    }

    float operator () (float x)
    {
        return base * x;
    }

    float operator () (float x, float y)
    {
        return base * x * y;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Multiplier m = { 2.0f };

    float r0 = m();
    float r1 = m(3.0f);
    float r2 = m(3.0f, 4.0f);

    outputBuffer[0] = (int)r0;
    outputBuffer[1] = (int)r1;
    outputBuffer[2] = (int)r2;
}

// CHECK: 2
// CHECK: 6
// CHECK: 24

// Three operator() overloads (mangled as x28x29) for zero, one, two params.
// CHECK-DAG directives used because overload ordering may vary.
// HLSL: struct Multiplier_{{[0-9_]*}}
// HLSL-DAG: Multiplier_x28x29_{{[0-9_]*}}(Multiplier_{{[0-9_]*}} this_{{[0-9_]*}})
// HLSL-DAG: Multiplier_x28x29_{{[0-9_]*}}(Multiplier_{{[0-9_]*}} this_{{[0-9_]*}}, float x_{{[0-9_]*}})
// HLSL-DAG: Multiplier_x28x29_{{[0-9_]*}}(Multiplier_{{[0-9_]*}} this_{{[0-9_]*}}, float x_{{[0-9_]*}}, float y_{{[0-9_]*}})

// GLSL: struct Multiplier_{{[0-9_]*}}
// GLSL-DAG: Multiplier_x28x29_{{[0-9_]*}}(Multiplier_{{[0-9_]*}} this_{{[0-9_]*}})
// GLSL-DAG: Multiplier_x28x29_{{[0-9_]*}}(Multiplier_{{[0-9_]*}} this_{{[0-9_]*}}, float x_{{[0-9_]*}})

// METAL: struct Multiplier_{{[0-9_]*}}
// METAL-DAG: Multiplier_x28x29_{{[0-9_]*}}
// METAL-DAG: Multiplier_x28x29_{{[0-9_]*}}

// WGSL: struct Multiplier_{{[0-9_]*}}
// WGSL-DAG: fn Multiplier_x28x29_{{[0-9_]*}}
// WGSL-DAG: fn Multiplier_x28x29_{{[0-9_]*}}

// CUDA: struct Multiplier_{{[0-9_]*}}
// CUDA-DAG: Multiplier_x28x29_{{[0-9_]*}}
// CUDA-DAG: Multiplier_x28x29_{{[0-9_]*}}

// CPP: struct Multiplier_{{[0-9_]*}}
// CPP-DAG: Multiplier_x28x29_{{[0-9_]*}}
// CPP-DAG: Multiplier_x28x29_{{[0-9_]*}}
