//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Forcing `Container.Element` may publish the nested `IBase` witness table before the outer
// `IDerived : IBase` requirement has been fully validated. The compilation must still diagnose
// that `requiredMethod` is absent when whole-conformance checking resumes.
interface IBase
{
    associatedtype Element;
    void requiredMethod();
//CHECK: ^^^^^^^^^^^^^^ see declaration of 'requiredMethod'
}

interface IDerived : IBase {}
//CHECK:             ^^^^^ missing interface member
//CHECK:             ^^^^^ type 'Container' does not provide required interface member 'requiredMethod'

struct Container : IDerived
{
    typealias Element = int;
}

struct RequiresIntElement<T : IDerived>
    where T.Element == int
{}

RequiresIntElement<Container> value;
