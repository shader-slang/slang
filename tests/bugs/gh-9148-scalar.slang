//DIAGNOSTIC_TEST:SIMPLE(filecheck=DIAG): -target spirv -O3
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -emit-spirv-directly -output-using-type

// Test for issue #9148: passing a pointer to a buffer element as a function
// parameter should not assert due to mismatched address spaces between
// the UserPointer address space and the specialized StorageBuffer address space.
// DIAG-NOT: field_address requires base and result to have same address space

struct Foo {
    int val;
};

//TEST_INPUT:ubuffer(data=[42 7], stride=4):name=buf
RWStructuredBuffer<Foo> buf;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int bar(Foo* f) {
    return f->val;
}

[numthreads(1, 1, 1)]
void computeMain() {
    outputBuffer[0] = bar(&buf[0]);
    outputBuffer[1] = bar(&buf[1]);
}

// CHECK: 42
// CHECK-NEXT: 7
