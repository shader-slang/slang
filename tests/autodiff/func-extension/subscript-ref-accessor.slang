//TEST:SIMPLE:-experimental-feature

// Verify that a ref subscript accessor can be named as a custom-derivative target.

struct RefCell : IDifferentiable
{
    typedef RefCell Differential;

    float value;

    [Differentiable]
    static Differential dzero()
    {
        return RefCell(0.0);
    }

    [Differentiable]
    static Differential dadd(Differential a, Differential b)
    {
        return RefCell(a.value + b.value);
    }

    __subscript(int index) -> float
    {
        ref { return value; }
    }
}

__func_extension fwd_diff(RefCell::__subscript::ref)(
    DifferentialPair<RefCell> self,
    int index) -> DifferentialPair<float>
{
    return diffPair(self.p[index], self.d[index]);
}

__func_extension bwd_diff(RefCell::__subscript::ref)(
    inout DifferentialPair<RefCell> self,
    int index,
    float dOut) -> void
{
    var dSelf = RefCell.dzero();
    dSelf[index] = dOut;
    self = diffPair(self.p, dSelf);
}

void testRefAccessorDerivativeReference()
{
    let primal = RefCell(2.0);
    let differential = RefCell(20.0);

    let getResult = fwd_diff(RefCell::__subscript::ref)(
        diffPair(primal, differential),
        1);

    var getSelf = diffPair(primal);
    bwd_diff(RefCell::__subscript::ref)(getSelf, 0, 7.0);
}
