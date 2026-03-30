//TEST:SIMPLE(filecheck=CHECK_O0): -target spirv -g2 -O0
//TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -g2 -O1
//TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -g2 -O2

// Test that nested struct local variable members have proper debug info
// that a shader debugger can resolve, both with and without optimizations
// enabled.

struct Inner
{
    float x;
    int y;
};

struct Outer
{
    Inner inner;
    float z;
};

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    Outer o;
    o.inner.x = 1.0;
    o.inner.y = int(tid);
    o.z = 3.0;
    output[0] = o.inner.x + float(o.inner.y) + o.z;
}

// -O0: struct uses both DebugDeclare and per-member DebugValue.
// CHECK_O0-DAG: DebugTypeComposite
// CHECK_O0-DAG: [[O_STR:%[0-9]+]] = OpString "o"
// CHECK_O0-DAG: [[O:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[O_STR]]
// CHECK_O0-DAG: DebugDeclare [[O]]
// CHECK_O0-DAG: DebugValue [[O]] {{.*}} %int_0 %int_0{{$}}
// CHECK_O0-DAG: DebugValue [[O]] {{.*}} %int_0 %int_1{{$}}
// CHECK_O0-DAG: DebugValue [[O]] {{.*}} %int_1{{$}}

// -O1/-O2: nested struct should use DebugValue per leaf member with
// multi-level access chain indices.
// CHECK_OPT-DAG: DebugTypeComposite
// CHECK_OPT-DAG: [[O_STR:%[0-9]+]] = OpString "o"
// CHECK_OPT-DAG: [[O:%[A-Za-z_0-9]+]] = OpExtInst %void {{.*}} DebugLocalVariable [[O_STR]]
// CHECK_OPT-NOT: DebugDeclare [[O]]
// CHECK_OPT-DAG: DebugValue [[O]] {{.*}} %int_0 %int_0{{$}}
// CHECK_OPT-DAG: DebugValue [[O]] {{.*}} %int_0 %int_1{{$}}
// CHECK_OPT-DAG: DebugValue [[O]] {{.*}} %int_1{{$}}
