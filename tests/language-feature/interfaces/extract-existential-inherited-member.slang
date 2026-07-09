//TEST:INTERPRET(filecheck=CHECK):

// Regression coverage for the ExtractExistentialType facet/witness re-rooting
// (#11464). Opening an existential of a *derived* interface (`IDerived`) and
// dispatching a requirement *inherited* from its base (`IBase`) exercises the
// re-rooting: the opened existential's inherited base facet must carry a witness
// rooted at the opened value, not at the interface's `This`. A non-canonical or
// mis-rooted inherited facet would mis-dispatch (or fail to intern) here.

interface IBase
{
    associatedtype D : IBase;
    int baseVal();
}

interface IDerived : IBase
{
    int derivedVal();
}

struct C : IDerived
{
    typealias D = C;
    int baseVal() { return 40; }
    int derivedVal() { return 2; }
}

// `obj` is an opened `IDerived` existential; `baseVal()` is the inherited `IBase`
// requirement dispatched through the re-rooted inherited facet.
int sumThroughExistential(IDerived obj)
{
    return obj.baseVal() + obj.derivedVal();
}

void main()
{
    C c;
    // CHECK: 42
    printf("%d\n", sumThroughExistential(c));
}
