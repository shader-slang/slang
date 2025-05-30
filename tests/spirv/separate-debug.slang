//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -g2 -emit-spirv-directly -separate-debug-info

struct TestType
{
    float memberA;
    float3 memberB;
    RWStructuredBuffer<float> memberC;
    float getValue()
    {
        return memberA;
    }
}
RWStructuredBuffer<float> result;
void main()
{
    TestType t;
    t.memberA = 1.0;
    t.memberB = float3(1, 2, 3);
    t.memberC = result;
    var val = t.getValue();
    result[0] = val + t.memberB.x;
}

// CHECK-NOT: OpExtInst %void {{.*}} DebugExpression
// CHECK-NOT: DebugTypeMember
// CHECK-NOT: DebugTypeComposite
// CHECK-NOT: DebugFunctionDefinition
// CHECK-NOT: DebugScope
// CHECK-NOT: DebugLine
// CHECK-NOT: DebugValue
