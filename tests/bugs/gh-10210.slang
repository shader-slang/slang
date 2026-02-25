//DIAGNOSTIC_TEST:SIMPLE: -target spirv -entry computeMain -stage compute

// Regression test: multi-interface existential dispatch dictionary crash
// Issue: https://github.com/shader-slang/slang/issues/10210

interface IBase { float base(); }
interface IDerived : IBase { float derived(); }

struct Impl : IDerived {
    float val;
    float base() { return val; }
    float derived() { return val * 2.0; }
};

RWStructuredBuffer<float> output;
float useBase(IBase b) { return b.base(); }
float useDerived(IDerived d) { return d.derived() + useBase(d); }

[numthreads(1,1,1)]
void computeMain() {
    Impl i;
    i.val = 1.0;
    output[0] = useDerived(i);
}
