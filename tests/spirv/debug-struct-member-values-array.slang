//TEST:SIMPLE(filecheck=CHECK_O0): -target spirv -g2 -O0
//TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -g2 -O1
//TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -g2 -O2

// Test that a struct with an array member has proper debug info.
// Exercises the array index path in translateIRAccessChain.

struct WithArray
{
    float a;
    float values[3];
};

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    WithArray s;
    s.a = 1.0;
    s.values[0] = 2.0;
    s.values[1] = 3.0;
    s.values[2] = float(tid);
    output[0] = s.a + s.values[0] + s.values[1] + s.values[2];
}

// -O0: struct uses both DebugDeclare and per-member DebugValue.
// CHECK_O0-DAG: DebugTypeArray
// CHECK_O0-DAG: DebugTypeComposite
// CHECK_O0-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// CHECK_O0-DAG: [[S:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[S_STR]]
// CHECK_O0-DAG: DebugDeclare [[S]]
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_0{{$}}
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_1 %int_0{{$}}
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_1 %int_1{{$}}
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_1 %int_2{{$}}

// -O1/-O2: per-member DebugValue with array access chain indices.
// CHECK_OPT-DAG: DebugTypeArray
// CHECK_OPT-DAG: DebugTypeComposite
// CHECK_OPT-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// CHECK_OPT-DAG: [[S:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[S_STR]]
// CHECK_OPT-NOT: DebugDeclare [[S]]
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_0{{$}}
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_1 %int_0{{$}}
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_1 %int_1{{$}}
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_1 %int_2{{$}}
