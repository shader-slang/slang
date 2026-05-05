//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -emit-spirv-directly -entry main -stage compute
// Verifies DebugLine columns are clamped to the embedded DebugSource text when #line maps
// space-indented generated source back to a tab-indented source file.

//CHECK-DAG: [[ORIGINAL_FILE:%[0-9]+]] = OpString "tests/spirv/debug-line-column-clamp-original.slang"
//CHECK-DAG: [[ORIGINAL_SOURCE:%[0-9]+]] = {{.*}} DebugSource [[ORIGINAL_FILE]]
//CHECK: DebugLine [[ORIGINAL_SOURCE]] %uint_5 %uint_5 %uint_3 %uint_3

#line 1 "tests/spirv/debug-line-column-clamp-original.slang"
RWStructuredBuffer<float> outputBuffer;

float helper(float value)
{
    {
    return value;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = helper(1.0);
}
