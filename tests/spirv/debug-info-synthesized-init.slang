//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -g2 -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=THIS):-target spirv-asm -g2 -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=LINES):-target spirv-asm -g2 -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=MINIMAL):-target spirv-asm -g1 -emit-spirv-directly -O0

// Check that generated constructors retain source locations and carry FlagArtificial,
// while user-written constructors remain unflagged. This metadata lets consumers
// choose their source-stepping policy.
//CHECK-DAG: [[PAIR:%[0-9]+]] = OpString "Pair.$init"
//CHECK-DAG: [[DEFAULT:%[0-9]+]] = OpString "DefaultMade.$init"
//CHECK-DAG: [[USER:%[0-9]+]] = OpString "UserMade.$init"
//CHECK: [[ARTIFICIAL:%[a-zA-Z0-9_]+]] = OpConstant %uint 32
//CHECK-DAG: [[PAIR_FUNC:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[PAIR]] {{.*}} [[PAIR]] [[ARTIFICIAL]]
//CHECK-DAG: [[DEFAULT_FUNC:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[DEFAULT]] {{.*}} [[DEFAULT]] [[ARTIFICIAL]]
//CHECK-DAG: [[USER_FUNC:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[USER]] {{.*}} [[USER]] %uint_0

// Check that generated and user-written constructors each expose a `this` variable
// scoped to their own debug function.
//THIS-DAG: [[PAIR:%[0-9]+]] = OpString "Pair.$init"
//THIS-DAG: [[DEFAULT:%[0-9]+]] = OpString "DefaultMade.$init"
//THIS-DAG: [[USER:%[0-9]+]] = OpString "UserMade.$init"
//THIS-DAG: [[THIS_STR:%[0-9]+]] = OpString "this"
//THIS-DAG: [[PAIR_FUNC:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[PAIR]]
//THIS-DAG: [[DEFAULT_FUNC:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[DEFAULT]]
//THIS-DAG: [[USER_FUNC:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[USER]]
//THIS-DAG: DebugLocalVariable [[THIS_STR]] {{.*}} [[PAIR_FUNC]] {{%uint_[0-9]+}}
//THIS-DAG: DebugLocalVariable [[THIS_STR]] {{.*}} [[DEFAULT_FUNC]] {{%uint_[0-9]+}}
//THIS-DAG: DebugLocalVariable [[THIS_STR]] {{.*}} [[USER_FUNC]] {{%uint_[0-9]+}}

// Check that full debug info retains constructor scopes and member locations, while
// minimal debug info omits generated member locations.
//LINES: [[PAIR:%[0-9]+]] = OpString "Pair.$init"
//LINES: [[PAIR_FUNC:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[PAIR]]
//LINES: DebugFunctionDefinition [[PAIR_FUNC]]
//LINES: DebugScope [[PAIR_FUNC]]

// Minimal debug info still emits OpLine for user code. The first OpLine belongs to `main`,
// which precedes the constructor bodies, so the MINIMAL-NOT checks below span every body.
//MINIMAL: OpLine {{%[0-9]+}} {{[0-9]+}} {{[0-9]+}}

struct Pair
{
    float x;
    //LINES: DebugLine {{.*}} %uint_[[#@LINE-1]]
    //MINIMAL-NOT: OpLine {{.*}} [[#@LINE-2]] {{[0-9]+}}
    float y;
    //LINES: DebugLine {{.*}} %uint_[[#@LINE-1]]
    //MINIMAL-NOT: OpLine {{.*}} [[#@LINE-2]] {{[0-9]+}}
};

struct UserMade
{
    float v;
    __init(float a)
    {
        v = a;
    }
};

struct DefaultMade
{
    float w;
    //MINIMAL-NOT: OpLine {{.*}} [[#@LINE-1]] {{[0-9]+}}
};

RWStructuredBuffer<float> outputBuffer;

// Check that caller statements retain their debug lines around constructor calls.
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Pair p = {1.0, 2.0};
    //CHECK: DebugLine {{.*}} %uint_[[#@LINE-1]]
    UserMade u = UserMade(3.0);
    DefaultMade d = {};
    outputBuffer[tid.x] = p.x + p.y + u.v + d.w;
    //CHECK: DebugLine {{.*}} %uint_[[#@LINE-1]]
}

//CHECK-DAG: DebugFunctionDefinition [[PAIR_FUNC]]
//CHECK-DAG: DebugFunctionDefinition [[DEFAULT_FUNC]]
//CHECK-DAG: DebugFunctionDefinition [[USER_FUNC]]
