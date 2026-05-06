// Test that `is`/`as` operators do NOT emit E30303 for parametric types
// (generic type parameters, nested generic args) and DO emit E30303 for
// unrelated concrete types.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Dog
{
    int bark;
}

struct Cat
{
    float meow;
}

struct Box<T>
{
    T value;
}

// Generic type parameter - should NOT trigger E30303
bool testGenericParam<T>(T val)
{
    let d = val as Dog;
    return val is Dog;
}
//CHECK-NOT: unrelated concrete types

// Nested generic arg - should NOT trigger E30303
bool testNestedGeneric<T>(Box<T> box)
{
    let dogBox = box as Box<Dog>;
    return box is Box<Dog>;
}
//CHECK-NOT: unrelated concrete types

// Unrelated concrete types - SHOULD trigger E30303
void testConcreteMismatch()
{
    Dog d;
    bool b = d is Cat;
//CHECK: 'is'/'as' on unrelated concrete types
//CHECK: 'is'/'as' on unrelated concrete types will never succeed.
    let c = d as Cat;
//CHECK: 'is'/'as' on unrelated concrete types
//CHECK: 'is'/'as' on unrelated concrete types will never succeed.
}
