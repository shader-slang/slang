//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// Companion to overload-generic-constraint-note (issue #12965): the same generic requirement is
// inherited through two specializations of a generic base interface, so overload resolution sees
// two candidates that share a declaration but differ by substitution. Each specialization is
// listed with its own unsatisfied-constraint note.

interface IBase<T>
{
    void doThing<U : IBase<T>>(U u);
}

interface IDerived : IBase<int>, IBase<float> {}

struct W {}

void call<D : IDerived>(D d)
{
    // `W` conforms to neither `IBase<int>` nor `IBase<float>`, so `doThing` fails for both
    // inherited specializations.
    d.doThing(W());
}

// One check per emitted diagnostic (SIMPLE mode is exhaustive). Both specializations are listed
// with their own substituted constraint.
//CHECK: no overload for 'doThing' applicable to arguments of type (W)
//CHECK: candidate: func D.doThing<U> -> void
//CHECK: type argument 'W' does not conform to the required interface 'IBase<int>'
//CHECK: candidate: func D.doThing<U> -> void
//CHECK: type argument 'W' does not conform to the required interface 'IBase<float>'
