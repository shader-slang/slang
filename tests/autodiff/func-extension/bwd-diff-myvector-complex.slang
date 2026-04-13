//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Complex test: A generic MyVector<T> type with arithmetic operations,
// where IDifferentiable conformance is added via extension only for
// floating-point instantiations with T.Differential == T.
// Explicit dzero/dadd avoid the need for DerivativeMemberAttribute on fields.
// Backward derivatives are provided via __func_extension.

struct MyVector<T : __BuiltinArithmeticType>
{
    T x;
    T y;

    // Element-wise square
    MyVector<T> squared()
    {
        return MyVector<T>(x * x, y * y);
    }

    // Dot product with another vector
    T dot(MyVector<T> b)
    {
        return x * b.x + y * b.y;
    }
};

// IDifferentiable conformance: only for floating-point element types.
// Provide explicit dzero/dadd so the compiler doesn't need to synthesize
// them from DerivativeMemberAttribute (which requires fields to be
// differentiable at the struct's definition site).
extension<T : __BuiltinFloatingPointType> MyVector<T> : IDifferentiable
    where T.Differential == T
{
    typedef MyVector<T> Differential;

    static Differential dzero()
    {
        return MyVector<T>(T(0), T(0));
    }

    static Differential dadd(Differential a, Differential b)
    {
        return MyVector<T>(a.x + b.x, a.y + b.y);
    }
}

// Element-wise exp: only available for floating-point element types.
extension<T : __BuiltinFloatingPointType> MyVector<T>
{
    MyVector<T> vexp()
    {
        return MyVector<T>(exp(x), exp(y));
    }
}

// --- Backward derivatives via __func_extension ---
// Only defined for __BuiltinFloatingPointType with T.Differential == T.

__func_extension<T : __BuiltinFloatingPointType>
bwd_diff(MyVector<T>::squared)(inout DifferentialPair<MyVector<T>> self, MyVector<T> dOut) -> void
    where T.Differential == T
{
    // d/d(self) of (x^2, y^2) = (2x*dOut.x, 2y*dOut.y)
    self = diffPair(self.p, MyVector<T>(T(2) * self.p.x * dOut.x, T(2) * self.p.y * dOut.y));
}

__func_extension<T : __BuiltinFloatingPointType>
bwd_diff(MyVector<T>::vexp)(inout DifferentialPair<MyVector<T>> self, MyVector<T> dOut) -> void
    where T.Differential == T
{
    // d/d(self) of (exp(x), exp(y)) = (exp(x)*dOut.x, exp(y)*dOut.y)
    self = diffPair(self.p, MyVector<T>(dOut.x * exp(self.p.x), dOut.y * exp(self.p.y)));
}

__func_extension<T : __BuiltinFloatingPointType>
bwd_diff(MyVector<T>::dot)(inout DifferentialPair<MyVector<T>> self, inout DifferentialPair<MyVector<T>> b, T dOut) -> void
    where T.Differential == T
{
    // d(self.dot(b))/d(self) = dOut * b, d(self.dot(b))/d(b) = dOut * self
    let px = self.p.x;
    let py = self.p.y;
    self = diffPair(self.p, MyVector<T>(dOut * b.p.x, dOut * b.p.y));
    b = diffPair(b.p, MyVector<T>(dOut * px, dOut * py));
}

// --- Compose the operations in a differentiable function ---

typedef MyVector<float> V;

// energy(pos, vel) = pos.squared().vexp().dot(vel)
[BackwardDifferentiable]
float computeEnergy(V pos, V vel)
{
    return pos.squared().vexp().dot(vel);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // pos=(1,0), vel=(1,1)
    // Primal: sq=(1,0), e=(e,1), dot(e,vel) = e+1 ~= 3.71828
    //
    // Backward chain rule with dOut=1:
    //   dot:     de=(1,1),     dvel=(e,1)
    //   vexp:    dsq=(e*1,1*1)=(e,1)
    //   squared: dpos=(2*1*e, 2*0*1) = (2e, 0)
    //
    // d/d(pos.x) = 2e ~= 5.43656
    // d/d(pos.y) = 0
    // d/d(vel.x) = e  ~= 2.71828
    // d/d(vel.y) = 1.0

    let pos = V(1.0, 0.0);
    let vel = V(1.0, 1.0);

    var dpPos = diffPair(pos, V(0.0, 0.0));
    var dpVel = diffPair(vel, V(0.0, 0.0));
    bwd_diff(computeEnergy)(dpPos, dpVel, 1.0);

    outputBuffer[0] = dpPos.d.x;
    outputBuffer[1] = dpPos.d.y;
    outputBuffer[2] = dpVel.d.x;
    outputBuffer[3] = dpVel.d.y;

    // CHECK: 5.43656
    // CHECK: 0.0
    // CHECK: 2.71828
    // CHECK: 1.0
}
