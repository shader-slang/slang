// Issue #13140: when several interfaces declare the missing member, the missing-constraint notes
// are emitted sorted by interface name, not in declaration or scope-walk order. `IZeta` is
// declared before `IAlpha`, and the notes still list `IAlpha` first.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

interface IZeta
{
    int ord();
}

interface IAlpha
{
    int ord();
}

int f<T>(T t)
{
    return t.ord();
}

// CHECK: error[E30027]: member not found
// CHECK: note[E30032]: consider adding a constraint such as 'where T : IAlpha'
// CHECK: note[E30032]: consider adding a constraint such as 'where T : IZeta'
