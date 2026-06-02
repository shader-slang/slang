//TEST:SIMPLE(filecheck=CHECK_O0): -target spirv -g2 -O0
//TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -g2 -O1
//TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -g2 -O2

// Test that a struct with a non-square matrix member has proper debug info.

struct WithMatrix
{
    float2x3 m;
    float a;
};

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    WithMatrix s;
    s.m[0][0] = 1.0;
    s.m[0][1] = 2.0;
    s.m[0][2] = 3.0;
    s.m[1][0] = 4.0;
    s.m[1][1] = 5.0;
    s.m[1][2] = float(tid);
    s.a = 7.0;
    output[0] =
        s.m[0][0] + s.m[0][1] + s.m[0][2] + s.m[1][0] + s.m[1][1] + s.m[1][2] + s.a;
}

// -O0: struct uses DebugDeclare and per-member DebugValue.
// CHECK_O0-DAG: DebugTypeMatrix
// CHECK_O0-DAG: DebugTypeComposite
// CHECK_O0-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// CHECK_O0-DAG: [[S:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[S_STR]]
// CHECK_O0-DAG: DebugDeclare [[S]]
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_0 %int_0 %int_0{{$}}
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_0 %int_0 %int_1{{$}}
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_0 %int_0 %int_2{{$}}
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_0 %int_1 %int_0{{$}}
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_0 %int_1 %int_1{{$}}
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_0 %int_1 %int_2{{$}}
// CHECK_O0-DAG: DebugValue [[S]] {{.*}} %int_1{{$}}

// -O1/-O2: struct should use per-member DebugValue, not DebugDeclare.
// CHECK_OPT-DAG: DebugTypeMatrix
// CHECK_OPT-DAG: DebugTypeComposite
// CHECK_OPT-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// CHECK_OPT-DAG: [[S:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[S_STR]]
// CHECK_OPT-NOT: DebugDeclare [[S]]
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_0 %int_0 %int_0{{$}}
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_0 %int_0 %int_1{{$}}
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_0 %int_0 %int_2{{$}}
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_0 %int_1 %int_0{{$}}
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_0 %int_1 %int_1{{$}}
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_0 %int_1 %int_2{{$}}
// CHECK_OPT-DAG: DebugValue [[S]] {{.*}} %int_1{{$}}
