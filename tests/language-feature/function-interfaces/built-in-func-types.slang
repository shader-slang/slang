//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test for synthesized associated types on functions

// Ideally what we'd like: 
/*interface IDifferentiableFuncBase
{
    __builtin_requirement(19)
    associatedtype __fwd_diff_func_type;
};

extension __func_as_type(zap) : IDifferentiableFuncBase
{ };

interface IForwardDifferentiable<functype FType>
{
    static __associatedfunc __fwd_diff_func_type<FType> __fwd_diff__;
}

float zap(float x)
{
    return 2 * x;
}

// Failed: cyclic reference can't be checked (but it's not a true cyclic reference)
//
// Can cyclic reference be fixed? No, because without expanding all the conformances, 
// we can't tell if __func_as_type(zap) : IDifferentiableFuncBase..
// 
// Needs a proper "on-demand" expansion of the facet list to check for cycles.
//
extension __func_as_type(zap) : IForwardDifferentiable<__func_type_of(zap)> 
{
    static DifferentialPair<float> __fwd_diff__(DifferentialPair<float> x)
    {
        return diffPair(x.p * x.p, 2 * x.d * x.p);
    }
}
*/

/*
__magic_type(ForwardDiffFuncType)
struct __fwd_diff_func_type<FType> { };

interface IForwardDifferentiable<FType>
{
    static __associatedfunc FType.__fwd_diff_func_type __fwd_diff__;
}

float zap(float x)
{
    return 2 * x;
}*/

// Will be in the core library
/*interface IDifferentiableFuncBase
{
    __builtin_requirement(19) // BuiltinRequirement::ForwardDiffFuncType
    associatedtype __fwd_diff_func_type;
};

// Will be auto-inserted.
extension __func_as_type(zap) : IDifferentiableFuncBase
{ };
*/

float zap(float x)
{
    return 2 * x;
}

extension __func_as_type(zap) : IDifferentiableFuncBase
{ /* __fwd_diff_func_type will be synthesized */ };

extension __func_as_type(zap) : IForwardDifferentiable<__func_as_type(zap)>
{
    
    static DifferentialPair<float> fwd_diff(DifferentialPair<float> x)
    {
        return diffPair(x.p * x.p, 2 * x.d * x.p);
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    outputBuffer[tig] = zap.fwd_diff(DifferentialPair<float>(0, 0)).d;
}
