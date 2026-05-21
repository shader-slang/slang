//TEST:INTERPRET(filecheck=CHECK):

// Regression test for resolving `This` inside a derived interface. The derived
// interface's own `This` declaration should shadow inherited `This.This`
// candidates from base-interface facets.

interface IBase
{
    This cloneBase();
}

interface IDerived : IBase
{
    This cloneDerived();
    int getValue();
}

interface IGrandDerived : IDerived
{
    This cloneGrandDerived();
}

struct Impl : IGrandDerived
{
    Impl cloneBase()
    {
        return this;
    }

    Impl cloneDerived()
    {
        return this;
    }

    Impl cloneGrandDerived()
    {
        return this;
    }

    int getValue()
    {
        return 29;
    }
}

__generic<T : IDerived>
int readDerived(T value)
{
    return value.cloneDerived().getValue() + value.cloneBase().getValue();
}

__generic<T : IGrandDerived>
int readGrandDerived(T value)
{
    return value.cloneGrandDerived().getValue() + value.cloneDerived().getValue() +
           value.cloneBase().getValue();
}

void main()
{
    Impl value;
    printf("%d\n", readDerived(value));
    // CHECK: 58

    printf("%d\n", readGrandDerived(value));
    // CHECK: 87
}
