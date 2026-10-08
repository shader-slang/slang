//META: generated=true
//META: model=gpt-6
//META: generated_at=2026-09-29T07:48:53+00:00
//META: source_commit=3bb522221d5db85fbdf8dcf441c05ffc3b801358
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=55e4138f7b51a582d48fae00eb4b7bca5a411cd2c22c90e424f145698370dc0a
//META: purpose=Loop statement: SPIRV emits OpLoopMerge marking the loop merge block where all threads reconverge after the loop exits.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a loop that runs different iteration counts per thread emits
// structured loop control on every back-end. Threads loop 2 or 3 times
// (divergence); once every thread exits, all reconverge at the post-loop point.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(4,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Different threads loop different numbers of times: divergence
    uint numLoops = 2 + (tid.x & 1);
    uint sum = 0;
    for (uint i = 0; i < numLoops; ++i)
    {
        sum += i;
    }
    // All threads exit the loop: reconvergence at the merge block
    output[tid.x] = sum;
}

// SPIRV: OpLoopMerge marks the block where all threads reconverge after the loop.
// CHECK: OpLoopMerge

// HLSL: for-loop lowered to a top-test infinite loop with a break; the code
// after the loop is the reconvergence point.
// CHECK_HLSL: for(;;)
// CHECK_HLSL: break

// GLSL: for-loop lowered to a top-test infinite loop with a break.
// CHECK_GLSL: for(;;)
// CHECK_GLSL: break

// Metal: for-loop lowered to a top-test infinite loop with a break.
// CHECK_METAL: for(;;)
// CHECK_METAL: break

// WGSL uses its native loop statement with the same explicit exit.
// CHECK_WGSL: loop
// CHECK_WGSL-NEXT: {
// CHECK_WGSL: break;

// CUDA: for-loop lowered to a top-test infinite loop with a break.
// CHECK_CUDA: for(;;)
// CHECK_CUDA: break

// C++: for-loop lowered to a top-test infinite loop with a break.
// CHECK_CPP: for(;;)
// CHECK_CPP: break
