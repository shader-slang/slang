//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/effective-this-param-overloaded-call.slang -no-codegen

// Overload resolution selects a member from its explicit parameter types without considering
// receiver mutability. Semantic checking then validates the selected member's receiver: an `int`
// argument selects the mutating overload and diagnoses an immutable receiver, while a `float`
// argument selects the non-mutating overload and succeeds. The same validation applies to explicit and implicit
// receivers, including when a lambda captures an otherwise-writable outer `this` or derived local
// by value. The derived case also checks that capture rewriting reaches the receiver below its
// super-type cast.

#pragma warning(disable: 30816) // Struct inheritance is required to exercise a super-type cast.

struct Counter
{
    void increment(int amount) {}
    void increment(float amount) {}

    void useWritableThis()
    {
        increment(1);
        this.increment(1);
    }

    [nonmutating]
    void useImplicitThis()
    {
        increment(1);
//CHECK:         ^ mutating method cannot be called on immutable value
//CHECK:         ^ mutating method 'increment' cannot be called on an immutable value
//CHECK:^^^^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    }

    [nonmutating]
    void useExplicitThis()
    {
        this.increment(1);
//CHECK:              ^ mutating method cannot be called on immutable value
//CHECK:              ^ mutating method 'increment' cannot be called on an immutable value
//CHECK:^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    }

    [nonmutating]
    void useParenthesizedExplicitThis()
    {
        (this.increment)(1);
//CHECK:                ^ mutating method cannot be called on immutable value
//CHECK:                ^ mutating method 'increment' cannot be called on an immutable value
//CHECK: ^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    }

    void useCapturedThis()
    {
        let call = () => increment(1);
//CHECK:                          ^ mutating method cannot be called on immutable value
//CHECK:                          ^ mutating method 'increment' cannot be called on an immutable value
//CHECK:                 ^^^^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
        call();
    }

    [mutating]
    int selectByArgumentType(int value) { return value; }

    [nonmutating]
    int selectByArgumentType(float value) { return int(value); }

    [nonmutating]
    int useSelectedMutatingOverload()
    {
        return selectByArgumentType(1);
//CHECK:                           ^ mutating method cannot be called on immutable value
//CHECK:                           ^ mutating method 'selectByArgumentType' cannot be called on an immutable value
//CHECK:       ^^^^^^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    }

    [nonmutating]
    int useSelectedNonmutatingOverload()
    {
        // The float argument selects the nonmutating overload without diagnosing the unselected
        // mutating overload; the int argument selects the mutating overload and is rejected.
        return this.selectByArgumentType(1.0);
    }

    [mutating]
    void selectGeneric<T>(int value) {}

    [nonmutating]
    void selectGeneric<T>(float value) {}

    [nonmutating]
    void useSelectedGenericOverloads()
    {
        // `effective-this-param-generic-overload-selection.slang` verifies that selecting the
        // nonmutating float overload does not diagnose the mutating int candidate while
        // materializing this generic application.
        selectGeneric<int>(1.0);
        selectGeneric<int>(1);
//CHECK:                  ^ mutating method cannot be called on immutable value
//CHECK:                  ^ mutating method 'selectGeneric' cannot be called on an immutable value
//CHECK:^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    }
};

struct BaseCounter
{
    void setValue(int value) {}
    void setValue(float value) {}
};

struct DerivedCounter : BaseCounter
{
    [nonmutating]
    void useInheritedOverload()
    {
        this.setValue(1);
//CHECK:             ^ mutating method cannot be called on immutable value
//CHECK:             ^ mutating method 'setValue' cannot be called on an immutable value
//CHECK:^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    }
};

void useCapturedInheritedOverload()
{
    DerivedCounter counter;
    let call = () => counter.setValue(1);
//CHECK:                             ^ mutating method cannot be called on immutable value
//CHECK:                             ^ mutating method 'setValue' cannot be called on an immutable value
//CHECK:             ^^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    call();
}
