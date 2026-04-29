// Test that empty structs are properly removed from CUDA output.
// Empty structs in C++ have sizeof==1, but Slang layout computes them as size 0.
// If an empty struct survives into emitted CUDA code, this mismatch causes
// incorrect field offsets and runtime crashes (e.g., in slangpy on CUDA).

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry compute_main -target cuda -line-directive-mode none

// Verify the empty struct definition is not emitted.
// CHECK-NOT: struct EmptyType

public struct EmptyType
{
}

public struct CallData
{
    EmptyType p;
    RWStructuredBuffer<float> _result;
}
ParameterBlock<CallData> call_data;

// CHECK: struct CallData
// Verify CallData does not contain EmptyType.
// CHECK-NOT: EmptyType
// CHECK: RWStructuredBuffer
// CHECK: };

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_main()
{
    call_data._result[0] = 1;
}
