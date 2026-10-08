//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

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

void mutateInheritedStorage<T : IInheritedStorage>(inout T value)
{
    value.inherited = 1;
}

void acceptNonmutatingWriteback<T : IInheritedStorage>(
    NonmutatingWritebackStorage<T> input)
{
    NonmutatingWritebackStorage<T> outer = input;
    let directSet = () => { outer.value.inherited = 1; };
    let inoutCall = () => { mutateInheritedStorage(outer.value); };
    directSet();
    inoutCall();
}

void rejectMutatingWriteback<T : IInheritedStorage>(MutatingWritebackStorage<T> input)
{
    MutatingWritebackStorage<T> outer = input;
    let directSet = () => { outer.value.inherited = 1; };
//CHECK:                                          ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                                          ^ mutating 'get' accessor cannot be called on an immutable value
//CHECK:                    ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    let inoutCall = () => { mutateInheritedStorage(outer.value); };
//CHECK:                                                 ^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:                                                 ^^^^^ mutating 'get' accessor cannot be called on an immutable value
//CHECK:                                           ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    directSet();
    inoutCall();
}
