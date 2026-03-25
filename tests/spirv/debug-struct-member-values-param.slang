//TEST:SIMPLE(filecheck=CHECK_O0): -target spirv -g2 -O0
//TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -g2 -O1
//TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -g2 -O2

// Test that a struct-typed by-value function parameter has proper debug
// info through the parameter initialization path.

struct MyStruct
{
    float a;
    int b;
};

RWStructuredBuffer<float> output;

float processStruct(MyStruct param)
{
    return param.a + float(param.b);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    MyStruct local;
    local.a = 1.0;
    local.b = int(tid);
    output[0] = processStruct(local);
}

// -O0: both the local and parameter structs use DebugDeclare and per-member DebugValue.
// CHECK_O0-DAG: [[LOCAL_STR:%[0-9]+]] = OpString "local"
// CHECK_O0-DAG: [[PARAM_STR:%[0-9]+]] = OpString "param"
// CHECK_O0-DAG: DebugTypeComposite
// CHECK_O0-DAG: [[LOCAL:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[LOCAL_STR]]
// CHECK_O0-DAG: [[PARAM:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[PARAM_STR]]
// CHECK_O0-DAG: DebugDeclare [[LOCAL]]
// CHECK_O0-DAG: DebugDeclare [[PARAM]]
// CHECK_O0-DAG: DebugValue [[LOCAL]] {{.*}} %int_0{{$}}
// CHECK_O0-DAG: DebugValue [[LOCAL]] {{.*}} %int_1{{$}}
// CHECK_O0-DAG: DebugValue [[PARAM]] {{.*}} %int_0{{$}}
// CHECK_O0-DAG: DebugValue [[PARAM]] {{.*}} %int_1{{$}}

// -O1/-O2: both the local and parameter structs use per-member DebugValue.
// CHECK_OPT-DAG: [[LOCAL_STR:%[0-9]+]] = OpString "local"
// CHECK_OPT-DAG: [[PARAM_STR:%[0-9]+]] = OpString "param"
// CHECK_OPT-DAG: DebugTypeComposite
// CHECK_OPT-DAG: [[LOCAL:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[LOCAL_STR]]
// CHECK_OPT-DAG: [[PARAM:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[PARAM_STR]]
// CHECK_OPT-NOT: DebugDeclare [[LOCAL]]
// CHECK_OPT-NOT: DebugDeclare [[PARAM]]
// CHECK_OPT-DAG: DebugValue [[LOCAL]] {{.*}} %int_0{{$}}
// CHECK_OPT-DAG: DebugValue [[LOCAL]] {{.*}} %int_1{{$}}
// CHECK_OPT-DAG: DebugValue [[PARAM]] {{.*}} %int_0{{$}}
// CHECK_OPT-DAG: DebugValue [[PARAM]] {{.*}} %int_1{{$}}
