//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -profile glsl_450 -conformance Impl:IFoo=0 -g2 -O0 -validate-ir
//TEST:SIMPLE(filecheck=OPT): -target spirv-asm -profile glsl_450 -conformance Impl:IFoo=0 -g2 -O1 -validate-ir

// Check that existential specialization preserves a generated constructor's artificial debug info.
//CHECK: [[NAME:%[0-9]+]] = OpString "Box.$init"
//CHECK: [[ARTIFICIAL:%[a-zA-Z0-9_]+]] = OpConstant %uint 32
//CHECK: DebugFunction [[NAME]] {{.*}} [[NAME]] [[ARTIFICIAL]]

// The unused constructor can be eliminated at O1; specialization must still complete.
//OPT: OpEntryPoint GLCompute %main "main"

interface IFoo
{
    int method();
}

struct Impl : IFoo
{
    int method() { return 1; }
}

struct Box
{
    Optional<IFoo> value;
}

RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Box box = {};
    if (let value = box.value)
        output[0] = value.method();
    else
        output[0] = 100;
}
