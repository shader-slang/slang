//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// When an interface requirement is accessed through a constrained value, semantic checking inserts
// an interface upcast and carries the enclosing property's getter and setter operations through it.

interface IInheritedStorage
{
    property int inherited
    {
        get;
        set;
    }
}

struct NonmutatingWritebackStorage<T : IInheritedStorage>
{
    T backing;

    property T value
    {
        [nonmutating] get { return backing; }
        [nonmutating] set {}
        [mutating]
        ref { return backing; }
    }
}

struct MutatingWritebackStorage<T : IInheritedStorage>
{
    static T external;
    T backing;

    property T value
    {
        [mutating] get { return backing; }
        [mutating] set { backing = newValue; }
        [nonmutating]
        ref { return external; }
    }
}

void acceptNonmutatingWriteback<T : IInheritedStorage>(
    NonmutatingWritebackStorage<T> input)
{
    let outer = input;
    outer.value.inherited = 1;
}

void rejectMutatingWriteback<T : IInheritedStorage>(MutatingWritebackStorage<T> input)
{
    let outer = input;
    outer.value.inherited = 1;
//CHECK:  ^^^^^ mutating method cannot be called on immutable value
//CHECK:  ^^^^^ mutating method 'get' cannot be called on an immutable value
}

