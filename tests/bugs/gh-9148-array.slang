//DIAGNOSTIC_TEST:SIMPLE(filecheck=DIAG): -target spirv -O3
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type

// Test for issue #9148: passing a pointer to a buffer element containing
// a fixed-size array as a function parameter should not assert due to
// mismatched field keys between logical and storage array wrapper structs.
//DIAG-NOT: assert failure: resultType

// Also should not assert due to mismatched address spaces.
//DIAG-NOT: field_address requires base and result to have same address space

struct Foo {
    int vals[4];
};

// TEST_INPUT:ubuffer(data=[10 20 30 40], stride=4):name=buf
RWStructuredBuffer<Foo> buf;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int bar(Foo* f) {
    return f->vals[0] + f->vals[3];
}

[numthreads(1, 1, 1)]
void computeMain() {
    outputBuffer[0] = bar(&buf[0]);
    outputBuffer[1] = (&buf[0])->vals[2];
}

// CHECK: 50
// CHECK-NEXT: 30
