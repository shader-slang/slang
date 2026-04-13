//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Complex test: A generic MyVector<T> type with arithmetic operations,
// where IDifferentiable conformance is added via extension only for
// floating-point instantiations with T.Differential == T.

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

// --- Forward derivatives via __func_extension ---
// Only defined for __BuiltinFloatingPointType with T.Differential == T.

__func_extension<T : __BuiltinFloatingPointType>
fwd_diff(MyVector<T>::squared)(DifferentialPair<MyVector<T>> self) -> DifferentialPair<MyVector<T>>
    where T.Differential == T
{
    // d/d(self) of (x^2, y^2) = (2x*dx, 2y*dy)
    let sq = self.p.squared();
    return diffPair(sq, MyVector<T>(T(2) * self.p.x * self.d.x, T(2) * self.p.y * self.d.y));
}

__func_extension<T : __BuiltinFloatingPointType>
fwd_diff(MyVector<T>::vexp)(DifferentialPair<MyVector<T>> self) -> DifferentialPair<MyVector<T>>
    where T.Differential == T
{
    // d/d(self) of (exp(x), exp(y)) = (exp(x)*dx, exp(y)*dy)
    let ev = self.p.vexp();
    return diffPair(ev, MyVector<T>(ev.x * self.d.x, ev.y * self.d.y));
}

__func_extension<T : __BuiltinFloatingPointType>
fwd_diff(MyVector<T>::dot)(DifferentialPair<MyVector<T>> self, DifferentialPair<MyVector<T>> b) -> DifferentialPair<T>
    where T.Differential == T
{
    // d(self.dot(b)) = dself.dot(b) + self.dot(db)
    let p = self.p.dot(b.p);
    let d = self.d.dot(b.p) + self.p.dot(b.d);
    return diffPair(p, d);
}

// --- Compose the operations in differentiable functions ---

typedef MyVector<float> V;

// energy(pos, vel) = pos.squared().vexp().dot(vel)
[ForwardDifferentiable]
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
    // Primal: sq=(1,0), e=(e,1), dot(e,vel) = e+1 ≈ 3.71828
    //
    // d/d(pos.x): chain rule through vsquare→vexp→dot
    //   dsq=(2,0), de=(2e,0), d(dot)=2e ≈ 5.43656

    let pos = V(1.0, 0.0);
    let vel = V(1.0, 1.0);

    var dpPos = diffPair(pos, V(1.0, 0.0));
    var dpVel = diffPair(vel, V(0.0, 0.0));
    let result = fwd_diff(computeEnergy)(dpPos, dpVel);

    outputBuffer[0] = result.p;
    outputBuffer[1] = result.d;

    // d/d(vel.x): only the dot product sees the perturbation
    //   de=(0,0), d(dot) = dot(e,(1,0)) = e ≈ 2.71828
    var dpPos2 = diffPair(pos, V(0.0, 0.0));
    var dpVel2 = diffPair(vel, V(1.0, 0.0));
    let result2 = fwd_diff(computeEnergy)(dpPos2, dpVel2);
    outputBuffer[2] = result2.d;

    // CHECK: 3.71828
    // CHECK: 5.43656
    // CHECK: 2.71828
}
