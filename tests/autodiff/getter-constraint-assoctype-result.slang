//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

interface IMLP<T, int In, int Out>
{
    associatedtype Parameters : IDifferentiablePtrType;

    property DifferentialPtrPair<Parameters> parametersDual { get; }
}

extension<T:__BuiltinFloatingPointType> T: IDifferentiablePtrType
{
    typealias Differential = T;
}

struct MLP<T, int In, int Out> : IMLP<T, In, Out> where T: __BuiltinFloatingPointType
{
    typealias Parameters = Array<T, In + Out>;

    property DifferentialPtrPair<Parameters> parametersDual
    {
        get
        {
            Array<T, In + Out> primal = {};
            Array<T, In + Out> diff = {};
            for (int i = 0; i < In + Out; i++)
            {
                primal[i] = T(i);
                diff[i] = T(i + In + Out);
            }
            var dparams = DifferentialPtrPair<Parameters>(primal, diff);
            return dparams;
        }
    }
}
//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    MLP<float, 1, 1> mlp;
    // CHECK: 0.0
    outputBuffer[0] =  mlp.parametersDual.p[0];
    // CHECK: 1.0
    outputBuffer[1] =  mlp.parametersDual.p[1];

    // CHECK: 2.0
    outputBuffer[2] =  mlp.parametersDual.d[0];
    // CHECK: 3.0
    outputBuffer[3] =  mlp.parametersDual.d[1];
}
