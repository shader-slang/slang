//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// Test that a generic function combining specialization constants with regular
// function parameters compiles correctly: mixed spec-const/non-spec-const
// expressions must NOT be hoisted as OpSpecConstantOp.
// Regression test for https://github.com/ggml-org/llama.cpp/pull/20451

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Bc and WorkGroupSize are spec constants; verify they appear as such.
// CHECK-DAG: %[[BC:[0-9A-Za-z_]+]] = OpSpecConstant %uint 32
// CHECK-DAG: %[[WGS:[0-9A-Za-z_]+]] = OpSpecConstant %uint 128

// The expression `Bc * HS / 4` mixes spec const Bc with function param HS,
// so it must NOT appear as OpSpecConstantOp but as regular instructions
// inside the function body.
// CHECK-NOT: OpSpecConstantOp {{.*}} UDiv
// CHECK-NOT: OpSpecConstantOp {{.*}} IMul

// CHECK: OpFunction
// CHECK: OpIMul %uint %[[BC]]
// CHECK: OpUDiv %uint

[vk::constant_id(0)] const uint Bc = 32;
[vk::constant_id(1)] const uint WorkGroupSize = 128;

interface ILoader {
    float load(uint idx);
};

struct SimpleLoader : ILoader {
    float load(uint idx) { return float(idx); }
};

float doWork<L : ILoader>(L loader, uint HS, uint tid) {
    float result = 0.0;
    for (uint idx = 0; idx < Bc * HS / 4; idx += WorkGroupSize) {
        uint d = (idx + tid) % (HS / 4);
        if (d < Bc) {
            result += loader.load(d);
        }
    }
    return result;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    SimpleLoader loader;
    // Bc=32, WorkGroupSize=128, HS=8, tid=0
    // Loop: Bc*HS/4 = 32*8/4 = 64, step=128 → one iteration (idx=0)
    // d = (0+0) % (8/4) = 0, 0 < 32 → result += load(0) = 0.0
    float r0 = doWork(loader, 8, 0);

    // Bc=32, WorkGroupSize=128, HS=16, tid=3
    // Loop: Bc*HS/4 = 32*16/4 = 128, step=128 → one iteration (idx=0)
    // d = (0+3) % (16/4) = 3 % 4 = 3, 3 < 32 → result += load(3) = 3.0
    float r1 = doWork(loader, 16, 3);

    // BUF: 0.0
    outputBuffer[0] = r0;
    // BUF-NEXT: 3.0
    outputBuffer[1] = r1;
    // BUF-NEXT: 32.0
    outputBuffer[2] = float(Bc);
    // BUF-NEXT: 128.0
    outputBuffer[3] = float(WorkGroupSize);
}
