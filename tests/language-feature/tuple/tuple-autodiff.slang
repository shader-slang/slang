
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typealias A = Tuple<float, float>;

[BackwardDifferentiable]
A f(A a)
{
    // Read/writes to local struct variables won't be SSA'd out by default.
    // The backward diff preparation pass will kick in to create temp vars for them.
    A aout;
    aout._1 = 2 * a._0;
    aout._1 = aout._1 + 2 * a._0;
    aout._0 = aout._1 + 5 * a._0;

    // The result should be equivalent to:
    /*
    A aout;
    var tmp = 2 * a.x;
    tmp = tmp + 2 * a.x;
    aout.y = tmp;
    aout.x = tmp + 5 * a.x;
    */
    return aout;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    A a = makeTuple(1.0, 2.0);
    var dpa = diffPair(a);

    A dout = makeTuple(1.0, 1.0);
    
    __bwd_diff(f)(dpa, dout);
    // CHECK: 13
    outputBuffer[0] = dpa.d._0; // Expect: 13
    // CHECK: 0
    outputBuffer[1] = dpa.d._1; // Expect: 0
}
