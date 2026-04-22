//TEST_DISABLED:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension bwd_diff for a non-static method of a generic differentiable type,
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
bwd_diff(MyVec<T>::lengthSquared)(inout DifferentialPair<MyVec<T>> self, T dOut) -> void
    where T.Differential == T
{
    self = diffPair(self.p, MyVec<T>(dOut * T(2) * self.p.x, dOut * T(2) * self.p.y));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    var dpv = diffPair(MyVec<float>(3.0, 4.0), MyVec<float>(0.0, 0.0));
    bwd_diff(MyVec<float>::lengthSquared)(dpv, 1.0);
    outputBuffer[0] = dpv.d.x;
    outputBuffer[1] = dpv.d.y;
    // CHECK: 6.0
    // CHECK: 8.0
}
