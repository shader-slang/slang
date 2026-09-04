//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0

// Test that generic struct variables and parameters have proper debug info
// (DebugTypeComposite + DebugLocalVariable + field-level DebugValue) when the
// specialization happens after the debug-info generation pass, and that a second
// invocation of insertDebugValueStore does not duplicate any debug records.
// Regression test for https://github.com/shader-slang/slang/issues/9162

struct GenericStruct<T>
{
    T x;
    T y;
};

// A non-entry-point helper that takes a GenericStruct<float> *parameter*, so the
// parameter idempotency path is exercised in addition to the local-variable path.
float useGenericStructParam(GenericStruct<float> g)
{
    return g.x + g.y;
}

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    GenericStruct<float> s;
    s.x = 1.0;
    s.y = 2.0;
    output[0] = s.x + s.y + useGenericStructParam(s);
}

// The specialized struct must appear as a DebugTypeComposite with two members.
// CHECK-DAG: DebugTypeMember
// CHECK-DAG: [[COMPOSITE:%[A-Za-z0-9_]+]] = OpExtInst {{.*}} DebugTypeComposite

// Capture string literals up front (these live in the constants section, before any
// CHECK advances the scanner position past them).
// CHECK-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// CHECK-DAG: [[G_STR:%[0-9]+]] = OpString "g"

// Local variable 's' must have exactly one DebugLocalVariable referencing [[COMPOSITE]].
// CHECK-DAG: [[S_VAR:%[A-Za-z0-9_]+]] = OpExtInst {{.*}} DebugLocalVariable [[S_STR]] [[COMPOSITE]]
// CHECK-NOT: OpExtInst {{.*}} DebugLocalVariable [[S_STR]]
// CHECK-DAG: DebugDeclare [[S_VAR]]

// Parameter 'g' in useGenericStructParam must have exactly one DebugLocalVariable.
// CHECK-DAG: [[G_VAR:%[A-Za-z0-9_]+]] = OpExtInst {{.*}} DebugLocalVariable [[G_STR]] [[COMPOSITE]]
// CHECK-NOT: OpExtInst {{.*}} DebugLocalVariable [[G_STR]]

// Field-level DebugValue updates must be emitted for s.x and s.y assignments.
// CHECK-DAG: DebugValue [[S_VAR]] %float_1
// CHECK-DAG: DebugValue [[S_VAR]] %float_2
