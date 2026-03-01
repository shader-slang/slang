//TEST_IGNORE_FILE:

// Regression test: extension variable declarations crash SPIRV backend
// Issue: https://github.com/shader-slang/slang/issues/10200
// Note: TEST_IGNORE_FILE because this currently crashes the SPIRV backend.
// When fixed, change to: //TEST:SIMPLE: -target spirv -entry computeMain -stage compute

struct MyStruct { int val; }

extension MyStruct {
    int extraField;  // spec says this is invalid, but parser accepts it
}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain() {
    MyStruct s;
    s.val = 1;
    s.extraField = 2;
}
