//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0

// Test that generic struct variables have proper debug info (DebugLocalVariable
// and DebugTypeComposite) even when the specialization happens after the
// debug-info generation pass.
// Regression test for https://github.com/shader-slang/slang/issues/9162

struct GenericStruct<T>
{
    T x;
    T y;
};

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    GenericStruct<float> s;
    s.x = 1.0;
    s.y = 2.0;
    output[0] = s.x + s.y;
}

// The specialized struct should appear as a DebugTypeComposite.
// CHECK-DAG: DebugTypeComposite
// The local variable 's' should have a DebugLocalVariable.
// CHECK-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// CHECK-DAG: DebugLocalVariable [[S_STR]]
