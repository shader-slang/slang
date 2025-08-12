//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main
RWStructuredBuffer<float> result;

[ForceInline]
float myF(inout int a, int b)
{
    return a + b;
}

[ForceInline]
float myF(inout uint a, uint b)
{
    return a - b;
}

[ForceInline]
T myGenF<T : __BuiltinIntegerType>(inout T a, T b)
{
    if (__isSignedInt<T>())
    {
        return a + b;
    }
    else
    {
        return a - b;
    }
}
// CHECK: result{{.*}}[int(0)] = 1
// CHECK: result{{.*}}[int(1)] = 2
// CHECK: result{{.*}}[int(2)] = 1
// CHECK: result{{.*}}[int(3)] = 2
[numthreads(1,1,1)]
void main()
{
    int ic = 1;
    uint a = 2;
    // call-site myF(      uint,   int)
    // overload: myF(inout int, int)            this needs uint->int   cost: 300 (kConversionCost_SameSizeUnsignedToSignedConversion)
    //           myF(inout uint, uint)          this needs int->uint   cost: 250 (kConversionCost_SignedToUnsignedConversion)
    result[0] = myF(a, ic);

    int b = 3;
    uint uc = 1;
    result[1] = myF(b, uc);
    // call-site myF(      int, uint)
    // overload: myF(inout int, int)            this needs uint->int   cost: 300 (kConversionCost_SameSizeUnsignedToSignedConversion)
    //           myF(inout uint, uint)          this needs int->uint   cost: 250 (kConversionCost_SignedToUnsignedConversion)

    result[2] = myGenF(a, ic);
    // call-site        myF(      uint,   int)
    // specialize to    myF(inout uint, uint)
    // The process will be as follows:
    // We would try to resolve the type constraints by finding the best T that is int or uint.
    // Then we would try to resolve between those two T's, we we will prefer T == uint, because int->uint is cheaper.
    // Note that our type constraints resolve won't take argument type into account, it only compares all the candidates
    // of T to decide which one is the best. (see: trySolveConstraintSystem for details)

    result[3] = myGenF(b, uc);
    // Same as above
}
