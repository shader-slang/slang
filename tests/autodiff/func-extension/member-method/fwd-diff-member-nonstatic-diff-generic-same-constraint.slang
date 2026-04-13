//TEST_DISABLED:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension fwd_diff for a non-static method of a generic differentiable type,
// where the struct and func_extension share the same constraint.
//
// KNOWN ISSUE: When a generic struct `S<T : IFloat> : IDifferentiable` declares
// `typedef S<T> Differential`, the synthesized `dzero`/`dadd` functions assign
// `T.dzero()` (returns T.Differential) to fields of type T, causing a type mismatch.
// This is a pre-existing IDifferentiable synthesis issue, not a func_extension issue.
// Workaround: use a wider constraint on the struct (e.g., __BuiltinArithmeticType).

struct MyVec<T : __BuiltinFloatingPointType> : IDifferentiable
{
    typedef MyVec<T> Differential;
    T x;
    T y;

    T lengthSquared() { return x * x + y * y; }
};

__func_extension<T : __BuiltinFloatingPointType>
fwd_diff(MyVec<T>::lengthSquared)(DifferentialPair<MyVec<T>> self) -> DifferentialPair<T>
    where T.Differential == T
{
    T p = self.p.lengthSquared();
    T d = T(2) * self.p.x * self.d.x + T(2) * self.p.y * self.d.y;
    return diffPair(p, d);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    var dpv = diffPair(MyVec<float>(3.0, 4.0), MyVec<float>(1.0, 1.0));
    DifferentialPair<float> result = fwd_diff(MyVec<float>::lengthSquared)(dpv);
    outputBuffer[0] = result.p;
    outputBuffer[1] = result.d;
    // CHECK: 25.0
    // CHECK: 14.0
}
