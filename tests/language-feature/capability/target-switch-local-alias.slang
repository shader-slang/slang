//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -capability spvShaderClockKHR

// A local alias does not inherit the enclosing function's capability contract. Otherwise X
// would add ShaderClock to the first case, making its requirements incomparable with the second.
[require(spvShaderClockKHR)]
public int readLocalAlias()
{
    typealias X = int;
    __target_switch
    {
    case spirv:
        return X(1);
    case spvBindlessTextureNV:
        return 2;
    }
}

RWStructuredBuffer<int> outputBuffer;

// CHECK: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = readLocalAlias();
}
