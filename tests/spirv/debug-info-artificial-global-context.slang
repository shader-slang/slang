//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir -g1 -validate-ir

// Check that a global first referenced by a generated constructor retains its declaration
// location at Minimal debug level.
//CHECK: let [[SOURCE:%[a-zA-Z0-9_]+]] {{.*}} = DebugSource("{{.*}}debug-info-artificial-global-context.slang"

struct Generated
{
    private float value = globalInput[0];
    float read() { return value; }
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Generated generated = {};
    outputBuffer[0] = generated.read();
}

RWStructuredBuffer<float> globalInput;
//CHECK: [DebugLocation([[SOURCE]], [[#@LINE-1]] : UInt, {{[0-9]+}} : UInt)]
//CHECK-NEXT: [nameHint("globalInput")]
//CHECK-NEXT: [export({{.*}})]
//CHECK-NEXT: let {{%[a-zA-Z0-9_]+}} {{.*}} = global_param
