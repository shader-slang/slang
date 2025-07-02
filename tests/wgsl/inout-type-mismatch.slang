//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry main -stage compute

// Test for issue #7362: Crash when using wrong type for inout parameter with WGSL target
// This test verifies that the shader compiles successfully without crashing.
// The fix changed IRPtrType to IRPtrTypeBase in slang-ir-lower-l-value-cast.cpp
// to handle implicit l-value casts properly.

void B(inout uint param)
{
    param = param + 10;
}

void A(inout int param)
{
    // This should work with implicit l-value cast from int to uint
    // Previously this would cause a segfault due to null pointer dereference
    B(param);
}

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int param = 5;
    A(param);
}

// The test passes if compilation succeeds and generates valid WGSL
// CHECK: @compute
// CHECK: fn main