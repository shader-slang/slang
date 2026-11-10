//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -g2 -O0 -emit-spirv-directly -validate-ir

// Check that rebuilding the DebugFunction after the constructor's type changes keeps the
// artificial decoration. The mutable global texture becomes an added constructor parameter.
//CHECK: [[NAME:%[0-9]+]] = OpString "Generated.$init"
//CHECK: [[ARTIFICIAL:%[a-zA-Z0-9_]+]] = OpConstant %uint 32
//CHECK: [[CTOR:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[NAME]] {{.*}} [[NAME]] [[ARTIFICIAL]]
//CHECK: DebugFunctionDefinition [[CTOR]]

Texture2D<float> inputTexture;
static Texture2D<float> selectedTexture;
RWStructuredBuffer<float> outputBuffer;

struct Generated
{
    private float value = selectedTexture.Load(int3(0, 0, 0));

    float read()
    {
        return value;
    }
};

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    selectedTexture = inputTexture;
    Generated value = {};
    outputBuffer[0] = value.read();
}
