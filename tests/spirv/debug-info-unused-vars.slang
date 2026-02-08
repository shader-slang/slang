//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -g2 -emit-spirv-directly -O0

// Test that unused variables & functions aren't removed when debug info is enabled.

void TestFunc()
{
    float2x3 m = float2x3(
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0
    );

    float a = m[1][0];
    float b = m[0][1];
    float c = a + b;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    TestFunc();
}

// CHECK: OpExtInst %void {{.*}} DebugExpression
// CHECK: DebugLocalVariable
// CHECK: DebugLine