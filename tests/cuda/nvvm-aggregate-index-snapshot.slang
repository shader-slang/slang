//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):name=inputs
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputs
// CHECK: 3215431
// CHECK-NEXT: 6548762
// CHECK-NEXT: 6543213
// CHECK-NEXT: 6548762
// CHECK-NEXT: 6543213
// CHECK-NEXT: 6548762
// CHECK-NEXT: 3215431
// CHECK-NEXT: 6548762
// CHECK-NEXT: 3215431
// CHECK-NEXT: 7654324
// CHECK-NEXT: 7654324
// CHECK-NEXT: 3215431
// CHECK-NEXT: 3215431
// CHECK-NEXT: 6548762
// CHECK-NEXT: 8765435
// CHECK-NEXT: 6548762
// CHECK-NEXT: 8765435
// CHECK-NEXT: 6548762
// CHECK-NEXT: 3215431
// CHECK-NEXT: 6548762
// CHECK-NEXT: 3215431
// CHECK-NEXT: 9876546
// CHECK-NEXT: 9876546
// CHECK-NEXT: 3215431

struct Stack
{
    float weights[2];
    float3 normals[2];
    float3 tangents[2];
};

// Give every slot and vector lane a distinct decimal digit before taking the snapshot.
Stack initialize()
{
    Stack stack = {};
    stack.weights[0] = 1;
    stack.weights[1] = 2;
    stack.normals[0] = float3(3, 4, 5);
    stack.normals[1] = float3(6, 7, 8);
    stack.tangents[0] = float3(1, 2, 3);
    stack.tangents[1] = float3(4, 5, 6);
    return stack;
}

// Replace only the runtime-selected slot, leaving the other slot and saved value intact.
void mutate(inout Stack stack, uint slot, uint tag)
{
    stack.weights[slot] = float(tag + 3);
    stack.normals[slot] = float3(tag + 1, tag + 2, tag + 3);
    stack.tangents[slot] = float3(tag + 4, tag + 5, tag + 6);
}

// Encode each scalar separately so every vector lane and the weight remain observable.
uint encode(Stack stack, uint slot)
{
    return uint(stack.weights[slot])
        + 10 * uint(stack.normals[slot].x)
        + 100 * uint(stack.normals[slot].y)
        + 1000 * uint(stack.normals[slot].z)
        + 10000 * uint(stack.tangents[slot].x)
        + 100000 * uint(stack.tangents[slot].y)
        + 1000000 * uint(stack.tangents[slot].z);
}

StructuredBuffer<uint> inputs;
RWStructuredBuffer<uint> outputs;
[numthreads(1, 1, 1)]
void computeMain()
{
    for (uint i = 0; i < 4; ++i)
    {
        uint tag = inputs[i];
        uint slot = tag & 1;
        Stack live = initialize();
        Stack saved = live;
        mutate(live, slot, tag);
        // Literal observations detect even a consistent wrong-slot mutation and dynamic read.
        outputs[6 * i] = encode(saved, 0);
        outputs[6 * i + 1] = encode(saved, 1);
        outputs[6 * i + 2] = encode(live, 0);
        outputs[6 * i + 3] = encode(live, 1);
        outputs[6 * i + 4] = encode(live, slot);
        outputs[6 * i + 5] = encode(live, slot ^ 1);
    }
}
