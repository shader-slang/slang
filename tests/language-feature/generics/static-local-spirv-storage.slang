// Regression test for shader-slang/slang#11651.
//
// A function-local `static` variable reached through a non-inlined helper that
// runs multiple times in one dispatch must keep its backing storage module-scope
// `Private` so its value persists across calls; it must NOT be demoted to
// per-call `Function` storage. `writeSingle` is forced out-of-line with
// `[noinline]` so the storage class is observable in the emitted SPIR-V (an
// inlined helper folds the variable into the entry point and would mask the bug).
//
//   SPV  - structural check on the emitted SPIR-V (CI-runnable without a GPU):
//          the `static` backing variable must remain `Private`.
//   CHECK - behavioral check on the output buffer. The SPIR-V-specific demotion
//          only changes results on the GPU (vk) path; (cpu) is the correct-by-
//          construction baseline. (The variadic-`expand` form of this bug is
//          additionally covered by tests/language-feature/generics/variadic-void.slang.)

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[noinline]
void writeSingle(float value)
{
    static int counter = 0;
    outputBuffer[counter++] = value;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 1.0
    // CHECK: 2.0
    // CHECK: 3.0
    // CHECK: 4.0
    writeSingle(1.0);
    writeSingle(2.0);
    writeSingle(3.0);
    writeSingle(4.0);
}

// SPV: %counter = OpVariable %_ptr_Private_int Private
