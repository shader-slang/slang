// Test that empty structs nested within other structs are properly removed from CUDA output.
// This tests the case where an empty struct is a member of a non-empty struct,
// which is itself a member of a ParameterBlock element type.

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry compute_main -target cuda -line-directive-mode none

public struct EmptyType
{
}

public struct DataType
{
    float value;
    EmptyType empty;
}

public struct CallData
{
    DataType d;
    RWStructuredBuffer<float> _result;
}
ParameterBlock<CallData> call_data;

// Verify EmptyType is not emitted as a separate struct.
// CHECK-NOT: struct EmptyType

// Verify DataType does not contain EmptyType field.
// CHECK: struct DataType
// CHECK-NOT: EmptyType
// CHECK: float
// CHECK: };

// Verify CallData struct is emitted correctly.
// CHECK: struct CallData
// CHECK: DataType
// CHECK: RWStructuredBuffer
// CHECK: };

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_main()
{
    call_data._result[0] = call_data.d.value;
}
