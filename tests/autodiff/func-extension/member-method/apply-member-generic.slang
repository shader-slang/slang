//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension __apply for a generic member method.
// The this parameter is implicit — the apply body accesses it via `this`.

struct MyVec<T : __BuiltinFloatingPointType> : IDifferentiable
    where T.Differential == T
{
    typedef MyVec<T> Differential;
    T x;
    T y;

    T dot(MyVec<T> other) { return x * other.x + y * other.y; }
};

struct DotCtx
{
    float ax;
    float ay;
    float bx;
    float by;
    void operator()(out MyVec<float> dSelf, out MyVec<float> dOther, float dOut)
    {
        dSelf = MyVec<float>(dOut * bx, dOut * by);
        dOther = MyVec<float>(dOut * ax, dOut * ay);
    }
};

typedef MyVec<float> V;

// apply for MyVec<float>::dot — `this` is implicit
__func_extension __apply(V::dot)(V other) -> Tuple<float, DotCtx>
{
    let result = this.dot(other); // this.x * other.x + this.y * other.y
    return makeTuple(result, DotCtx(this.x, this.y, other.x, other.y));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let a = V(2.0, 3.0);
    let b = V(4.0, 5.0);
    // dot(a,b) = 2*4 + 3*5 = 23
    // d/d(a) = (4, 5), d/d(b) = (2, 3)
    var dpA = diffPair(a, V(0.0, 0.0));
    var dpB = diffPair(b, V(0.0, 0.0));
    bwd_diff(V::dot)(dpA, dpB, 1.0);
    outputBuffer[0] = dpA.d.x;
    outputBuffer[1] = dpA.d.y;
    outputBuffer[2] = dpB.d.x;
    outputBuffer[3] = dpB.d.y;
    // CHECK: 4.0
    // CHECK: 5.0
    // CHECK: 2.0
    // CHECK: 3.0
}
