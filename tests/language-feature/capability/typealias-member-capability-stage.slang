// A member alias propagates stage requirements as well as target requirements.
// Regression for https://github.com/shader-slang/slang/issues/13793.
//DIAGNOSTIC_TEST:SIMPLE(diag=STAGE,non-exhaustive): -target hlsl -entry stageMain -stage compute -no-codegen
//TEST:SIMPLE: -target hlsl -entry fragmentMain -stage fragment -no-codegen

struct Input<T>
{
}

// Explicit stage requirements on aliased types apply just like target requirements.
[require(fragment)]
struct FragmentOnly
{
}

struct FragmentContext
{
    typealias Element = FragmentOnly;
}

[numthreads(1, 1, 1)]
void stageMain(uniform Input<FragmentContext> input)
/*STAGE:
     ^^^^^^^^^ error E36107
*/
{
}

void fragmentMain(uniform Input<FragmentContext> input)
{
}
