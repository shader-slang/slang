//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -DIF
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -minimum-slang-optimization -DIF
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -DSWITCH
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -minimum-slang-optimization -DSWITCH

// CHECK: OpEntryPoint
// CHECK-NOT: error

// Resolution of `constantParamFunc` requires clean up of dead-branches,
// otherwise `static_assert` will incorrectly throw.

int constantParamFunc(constexpr int val)
{
#if defined(IF)
    if (val == 0)
    {
        static_assert(false, "fails here because of a bug");
        return 0;
    }
#elif defined(SWITCH)
    switch (val)
    {
    case 1:
        return 1;
    default:
        static_assert(false, "fails here because of a bug");
        return 0;
    }
#endif
    return 0;
}

[numthreads(1,1,1)]
void computeMain()
{
    constantParamFunc(1);
}