//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -experimental-feature

// A ref subscript accessor can be named as a custom-derivative target, but differentiating an
// actual indexing expression through it is not supported until ref accessor returns have a
// consistent by-reference representation in the AST and IR.

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

[Differentiable]
float readRefCell(RefCell value, int index)
{
    return value[index];
/*CHECK:
    ^^^^^^ cannot differentiate through ref accessor
    ^^^^^^ derivative cannot be propagated through a user-defined ref accessor because differentiable by-reference returns are not supported.
*/
}

[numthreads(1, 1, 1)]
void computeMain()
{
    let primal = RefCell(2.0);
    let differential = RefCell(20.0);

    fwd_diff(readRefCell)(diffPair(primal, differential), 1);
}
