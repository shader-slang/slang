//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

[anyValueSize(16)]
interface IMutable
{
    [mutating] void mutate();
}

struct MutableImpl : IMutable
{
    int value;
    [mutating] void mutate() { value++; }
}

struct Outer
{
    IMutable backing;

    property IMutable mutatingReadStorage
    {
        [mutating] get { return backing; }
        [nonmutating] set {}
    }

    property IMutable nonmutatingWritebackStorage
    {
        [nonmutating] get { return backing; }
        [nonmutating] set {}
    }
}

void testExistentialStorage(Outer input)
{
    Outer outer = input;

    // Opening the existential reads the property before invoking the requirement, so this call
    // cannot use the mutating getter on the immutable captured receiver.
    let rejectMutatingRead = () => { outer.mutatingReadStorage.mutate(); };
//CHECK:                                                             ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                                                             ^ mutating 'get' accessor cannot be called on an immutable value
//CHECK:                             ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

    // The opened value is written back through the nonmutating getter/setter pair.
    let acceptNonmutatingWriteback = () => { outer.nonmutatingWritebackStorage.mutate(); };

    rejectMutatingRead();
    acceptNonmutatingWriteback();
}
