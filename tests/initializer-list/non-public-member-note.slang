//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test 1: private member in public struct blocks initializer list; note names the member.
public struct HasPrivateMember
{
    private int data;
}

// Test 2: internal member in public struct also blocks initializer list; note names the member.
public struct HasInternalMember
{
    int data; // implicitly internal
}

// Test 3: non-interface inheritance blocks initializer list for MyDerived.
// The visibility note for MyDerived is suppressed because inheritance is the root cause.
// However, as a cascade, the compiler also tries to initialize the base class MyBase from
// the initializer list; that fails too and emits a note for MyBase's internal member.
public struct MyBase { int x; }

public struct MyDerived : MyBase
//CHECK:                  ^^^^^^ support for inheritance is unstable and will be removed in future language versions, consider using composition instead
{
    private int data;
}

// Test 4: explicit constructor (not the visibility mismatch) is the real reason
// the initializer list is rejected; visibility note must be suppressed.
public struct HasExplicitCtorAndPrivateMember
{
    private int x;
    __init(int a) { x = a; }
}

// Test 5: private member in internal struct blocks initializer list; note names the member.
internal struct HasPrivateMemberInternal
{
    private int data;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Should error: private member in public struct.
    HasPrivateMember a = HasPrivateMember({ 1 });
//CHECK: cannot use initializer list for type 'HasPrivateMember'
//CHECK: member 'data' is private, but 'HasPrivateMember' is public; all members must be public to use an initializer list

    // Should error: internal member in public struct.
    HasInternalMember b = HasInternalMember({ 2 });
//CHECK: cannot use initializer list for type 'HasInternalMember'
//CHECK: member 'data' is internal, but 'HasInternalMember' is public; all members must be public to use an initializer list

    // Should error: private member in internal struct.
    HasPrivateMemberInternal c = HasPrivateMemberInternal({ 3 });
//CHECK: cannot use initializer list for type 'HasPrivateMemberInternal'
//CHECK: member 'data' is private, but 'HasPrivateMemberInternal' is internal; all members must be internal to use an initializer list
}

[numthreads(1, 1, 1)]
void computeMain2()
{
    // Should error: inheritance (not visibility mismatch) is the root cause for MyDerived;
    // note is suppressed for MyDerived. Cascade also errors on MyBase.
    MyDerived d = MyDerived({ 1 });
//CHECK: cannot use initializer list for type 'MyDerived'
//CHECK-NOT: member 'data' is private, but 'MyDerived' is public
//CHECK: cannot use initializer list for type 'MyBase'
//CHECK: member 'x' is internal, but 'MyBase' is public; all members must be public to use an initializer list
}

[numthreads(1, 1, 1)]
void computeMain3()
{
    // Should error: ctor takes 1 arg, 2 were given.
    // Visibility note must NOT appear: the explicit ctor is the root cause, not
    // the member visibility mismatch.
    HasExplicitCtorAndPrivateMember h = HasExplicitCtorAndPrivateMember({ 1, 2 });
//CHECK: too many initializers in initializer list
//CHECK: too many initializers (expected 1, got 2)
//CHECK-NOT: member 'x' is private, but 'HasExplicitCtorAndPrivateMember' is public
}
