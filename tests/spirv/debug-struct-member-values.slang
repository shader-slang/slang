//TEST:SIMPLE(filecheck=CHECK_O0): -target spirv -g2 -O0
//TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -g2 -O1
//TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -g2 -O2

// Test that struct local variable members have proper debug info that a shader
// debugger can resolve, both with and without optimizations enabled.

struct MyStruct
{
    float a;
    float3 b;
    int c;
};

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    MyStruct s;
    s.a = 1.0;
    s.b = float3(2.0, 3.0, 4.0);
    s.c = int(tid);
    output[0] = s.a + s.b.x + float(s.c);
}

// -O0: struct uses both DebugDeclare and per-member DebugValue.
// CHECK_O0-DAG: DebugTypeMember
// CHECK_O0-DAG: DebugTypeComposite
// CHECK_O0-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// CHECK_O0-DAG: [[S:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[S_STR]]
// CHECK_O0-DAG: DebugDeclare [[S]]
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_0{{$}}
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_1{{$}}
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_2{{$}}

// -O1/-O2: struct should use per-member DebugValue, not DebugDeclare.
// CHECK_OPT-DAG: DebugTypeMember
// CHECK_OPT-DAG: DebugTypeComposite
// CHECK_OPT-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// CHECK_OPT-DAG: [[S:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[S_STR]]
// CHECK_OPT-NOT: DebugDeclare [[S]]
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_0{{$}}
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_1{{$}}
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_2{{$}}
